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

#include "chrdevtab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "range.h"

void
fildes_chrdevtab_init(struct fildes_chrdevtab* self,
                      struct picotm_error* error)
{
    self->len = 0;

    int err = pthread_rwlock_init(&self->rwlock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static size_t
chrdev_uninit_walk_cb(void* chrdev, struct picotm_error* error)
{
    chrdev_uninit(chrdev);
    return 1;
}

void
fildes_chrdevtab_uninit(struct fildes_chrdevtab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, self->len, sizeof(self->tab[0]),
                     chrdev_uninit_walk_cb, &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_chrdevtab(struct fildes_chrdevtab* chrdevtab,
                 struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&chrdevtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_chrdevtab(struct fildes_chrdevtab* chrdevtab,
                 struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&chrdevtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_chrdevtab(struct fildes_chrdevtab* chrdevtab)
{
    do {
        int err = pthread_rwlock_unlock(&chrdevtab->rwlock);
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
static struct chrdev*
append_empty_chrdev(struct fildes_chrdevtab* chrdevtab,
                    struct picotm_error* error)
{
    if (chrdevtab->len == picotm_arraylen(chrdevtab->tab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct chrdev* chrdev = chrdevtab->tab + chrdevtab->len;

    chrdev_init(chrdev, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++chrdevtab->len;

    return chrdev;
}

/* requires reader lock */
static struct chrdev*
find_by_id(struct fildes_chrdevtab* chrdevtab, const struct file_id* id)
{
    struct chrdev *chrdev_beg = picotm_arraybeg(chrdevtab->tab);
    const struct chrdev* chrdev_end = picotm_arrayat(chrdevtab->tab,
                                                     chrdevtab->len);

    while (chrdev_beg < chrdev_end) {

        const int cmp = file_ref_if_id(&chrdev_beg->base, id);
        if (!cmp) {
            return chrdev_beg;
        }

        ++chrdev_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct chrdev*
search_by_id(struct fildes_chrdevtab* chrdevtab, const struct file_id* id,
             int fildes, struct picotm_error* error)
{
    struct chrdev* chrdev_beg = picotm_arraybeg(chrdevtab->tab);
    const struct chrdev* chrdev_end = picotm_arrayat(chrdevtab->tab,
                                                     chrdevtab->len);

    while (chrdev_beg < chrdev_end) {

        const int cmp = file_ref_or_set_up_if_id(&chrdev_beg->base,
                                                 fildes, id, error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return chrdev_beg; /* set-up chrdev structure; return */
        }

        ++chrdev_beg;
    }

    return NULL;
}

struct chrdev*
fildes_chrdevtab_ref_fildes(struct fildes_chrdevtab* self, int fildes,
                            struct picotm_error* error)
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing chrdev structure with the given id.
     */

    rdlock_chrdevtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct chrdev* chrdev = find_by_id(self, &id);
    if (chrdev) {
        goto unlock; /* found chrdev for id; return */
    }

    unlock_chrdevtab(self);

    /* Not found entry; acquire writer lock to create a new entry in
     * the chrdev table.
     */

    wrlock_chrdevtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    chrdev = find_by_id(self, &id);
    if (chrdev) {
        goto unlock; /* found chrdev for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, chrdev structure.
     */

    struct file_id empty_id;
    file_id_clear(&empty_id);

    chrdev = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (chrdev) {
        goto unlock; /* found chrdev for id; return */
    }

    /* The chrdev table is full; create a new entry for the chrdev id at the
     * end of the table.
     */

    chrdev = append_empty_chrdev(self, error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_chrdev;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    file_ref_or_set_up(&chrdev->base, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_chrdev_ref_or_set_up;
    }

unlock:
    unlock_chrdevtab(self);

    return chrdev;

err_chrdev_ref_or_set_up:
err_append_empty_chrdev:
err_search_by_id:
    unlock_chrdevtab(self);
    return NULL;
}

size_t
fildes_chrdevtab_index(struct fildes_chrdevtab* self, struct chrdev* chrdev)
{
    return chrdev - self->tab;
}
