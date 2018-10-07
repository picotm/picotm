/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
find_by_id(struct fildes_dirtab* dirtab, const struct file_id* id)
{
    struct dir *dir_beg = picotm_arraybeg(dirtab->tab);
    const struct dir* dir_end = picotm_arrayat(dirtab->tab, dirtab->len);

    while (dir_beg < dir_end) {

        const int cmp = dir_cmp_and_ref(dir_beg, id);
        if (!cmp) {
            return dir_beg;
        }

        ++dir_beg;
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

        const int cmp = dir_cmp_and_ref_or_set_up(dir_beg, id, fildes,
                                                  error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return dir_beg; /* set-up dir structure; return */
        }

        ++dir_beg;
    }

    return NULL;
}

struct dir*
fildes_dirtab_ref_fildes(struct fildes_dirtab* self, int fildes,
                         struct picotm_error* error)
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing dir structure with the given id.
     */

    rdlock_dirtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct dir* dir = find_by_id(self, &id);
    if (dir) {
        goto unlock; /* found dir for id; return */
    }

    unlock_dirtab(self);

    /* Not found entry; acquire writer lock to create a new entry in
     * the dir table.
     */
    wrlock_dirtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    dir = find_by_id(self, &id);
    if (dir) {
        goto unlock; /* found dir for id; return */
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
    dir_ref_or_set_up(dir, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_dir_ref_or_set_up;
    }

unlock:
    unlock_dirtab(self);

    return dir;

err_dir_ref_or_set_up:
err_append_empty_dir:
err_search_by_id:
    unlock_dirtab(self);
    return NULL;
}

size_t
fildes_dirtab_index(struct fildes_dirtab* self, struct dir* dir)
{
    return dir - self->tab;
}
