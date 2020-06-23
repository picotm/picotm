/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 * Copyright (c) 2020       Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "dirtab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include <string.h>
#include "range.h"

void
fildes_dirtab_init(struct fildes_dirtab* self, struct picotm_error* error)
{
    self->len = 0;

    int err = pthread_rwlock_init(&self->rwlock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static size_t
dir_uninit_walk_cb(void* dir, struct picotm_error* error)
{
    dir_uninit(dir);
    return 1;
}

void
fildes_dirtab_uninit(struct fildes_dirtab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, self->len, sizeof(self->tab[0]),
                     dir_uninit_walk_cb, &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_dirtab(struct fildes_dirtab* dirtab, struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&dirtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_dirtab(struct fildes_dirtab* dirtab, struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&dirtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_dirtab(struct fildes_dirtab* dirtab)
{
    do {
        int err = pthread_rwlock_unlock(&dirtab->rwlock);
        if (err) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}

/* requires a writer lock */
static struct dir*
append_empty_dir(struct fildes_dirtab* dirtab, struct picotm_error* error)
{
    if (dirtab->len == picotm_arraylen(dirtab->tab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct dir* dir = dirtab->tab + dirtab->len;

    dir_init(dir, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++dirtab->len;

    return dir;
}

/* requires reader lock */
static struct dir*
find_by_id(struct fildes_dirtab* dirtab, const struct file_id* id,
           bool new_file, struct picotm_error* error)
{
    struct picotm_error saved_error = PICOTM_ERROR_INITIALIZER;

    struct dir *dir_beg = picotm_arraybeg(dirtab->tab);
    const struct dir* dir_end = picotm_arrayat(dirtab->tab, dirtab->len);

    while (dir_beg < dir_end) {

        struct picotm_error cmp_error = PICOTM_ERROR_INITIALIZER;

        const int cmp = file_ref_if_id(&dir_beg->base, id, new_file,
                                       &cmp_error);
        if (picotm_error_is_set(&cmp_error)) {
            /* An error might be reported if the id's file descriptors don't
             * match. We save the error, but continue the loops. If we later
             * find a full match, the function succeeds. Otherwise, it reports
             * the last error. */
            memcpy(&saved_error, &cmp_error, sizeof(saved_error));
        } else if (!cmp) {
            return dir_beg;
        }

        ++dir_beg;
    }

    if (picotm_error_is_set(&saved_error)) {
        memcpy(error, &saved_error, sizeof(*error));
    }

    return NULL;
}

/* requires writer lock */
static struct dir*
search_by_id(struct fildes_dirtab* dirtab, const struct file_id* id,
             int fildes, struct picotm_error* error)
{
    struct dir* dir_beg = picotm_arraybeg(dirtab->tab);
    const struct dir* dir_end = picotm_arrayat(dirtab->tab, dirtab->len);

    while (dir_beg < dir_end) {

        const int cmp = file_ref_or_set_up_if_id(&dir_beg->base,
                                                 fildes, false, id,
                                                 error);
        if (picotm_error_is_set(error)) {
            return NULL;
        } else if (!cmp) {
            return dir_beg; /* set-up dir structure; return */
        }

        ++dir_beg;
    }

    return NULL;
}

static bool
error_ne_fildes(bool new_file)
{
    return !new_file;
}

struct dir*
fildes_dirtab_ref_fildes(struct fildes_dirtab* self, int fildes,
                         bool new_file, struct picotm_error* error)
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct dir* dir;

    /* Try to find an existing dir structure with the given id; iff
     * a new element was not explicitly requested.
     */

    if (!new_file) {
        rdlock_dirtab(self, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }

        dir = find_by_id(self, &id, error_ne_fildes(new_file), error);
        if (picotm_error_is_set(error)) {
            goto err_find_by_id_1;
        } else if (dir) {
            goto unlock; /* found dir for id; return */
        }

        unlock_dirtab(self);
    }

    /* Not found or new entry is requested; acquire writer lock to
     * create a new entry in the dir table. */
    wrlock_dirtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    if (!new_file) {
        /* Re-try find operation; maybe element was added meanwhile. */
        dir = find_by_id(self, &id, error_ne_fildes(new_file), error);
        if (picotm_error_is_set(error)) {
            goto err_find_by_id_2;
        } else if (dir) {
            goto unlock; /* found dir for id; return */
        }
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, dir structure.
     */

    struct file_id empty_id;
    file_id_clear(&empty_id);

    dir = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (dir) {
        goto unlock; /* found dir for id; return */
    }

    /* The dir table is full; create a new entry for the dir id at the
     * end of the table.
     */

    dir = append_empty_dir(self, error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_dir;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    file_ref_or_set_up(&dir->base, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_dir_ref_or_set_up;
    }

unlock:
    unlock_dirtab(self);

    return dir;

err_dir_ref_or_set_up:
err_append_empty_dir:
err_search_by_id:
err_find_by_id_2:
err_find_by_id_1:
    unlock_dirtab(self);
    return NULL;
}

size_t
fildes_dirtab_index(struct fildes_dirtab* self, struct dir* dir)
{
    return dir - self->tab;
}
