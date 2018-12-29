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

#include "fifotab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "range.h"

void
fildes_fifotab_init(struct fildes_fifotab* self, struct picotm_error* error)
{
    self->len = 0;

    int err = pthread_rwlock_init(&self->rwlock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static size_t
fifo_uninit_walk_cb(void* fifo, struct picotm_error* error)
{
    fifo_uninit(fifo);
    return 1;
}

void
fildes_fifotab_uninit(struct fildes_fifotab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, self->len, sizeof(self->tab[0]),
                     fifo_uninit_walk_cb, &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_fifotab(struct fildes_fifotab* fifotab, struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&fifotab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_fifotab(struct fildes_fifotab* fifotab, struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&fifotab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_fifotab(struct fildes_fifotab* fifotab)
{
    do {
        int err = pthread_rwlock_unlock(&fifotab->rwlock);
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
static struct fifo*
append_empty_fifo(struct fildes_fifotab* fifotab, struct picotm_error* error)
{
    if (fifotab->len == picotm_arraylen(fifotab->tab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct fifo* fifo = fifotab->tab + fifotab->len;

    fifo_init(fifo, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++fifotab->len;

    return fifo;
}

/* requires reader lock */
static struct fifo*
find_by_id(struct fildes_fifotab* fifotab, const struct file_id* id)
{
    struct fifo *fifo_beg = picotm_arraybeg(fifotab->tab);
    const struct fifo* fifo_end = picotm_arrayat(fifotab->tab, fifotab->len);

    while (fifo_beg < fifo_end) {

        const int cmp = file_ref_if_id(&fifo_beg->base, id);
        if (!cmp) {
            return fifo_beg;
        }

        ++fifo_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct fifo*
search_by_id(struct fildes_fifotab* fifotab, const struct file_id* id,
             int fildes, struct picotm_error* error)
{
    struct fifo* fifo_beg = picotm_arraybeg(fifotab->tab);
    const struct fifo* fifo_end = picotm_arrayat(fifotab->tab, fifotab->len);

    while (fifo_beg < fifo_end) {

        const int cmp = file_ref_or_set_up_if_id(&fifo_beg->base,
                                                 fildes, id, error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return fifo_beg; /* set-up fifo structure; return */
        }

        ++fifo_beg;
    }

    return NULL;
}

struct fifo*
fildes_fifotab_ref_fildes(struct fildes_fifotab* self, int fildes,
                          struct picotm_error* error)
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing fifo structure with the given id.
     */

    rdlock_fifotab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fifo* fifo = find_by_id(self, &id);
    if (fifo) {
        goto unlock; /* found fifo for id; return */
    }

    unlock_fifotab(self);

    /* Not found entry; acquire writer lock to create a new entry in
     * the fifo table.
     */

    wrlock_fifotab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    fifo = find_by_id(self, &id);
    if (fifo) {
        goto unlock; /* found fifo for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, fifo structure.
     */

    struct file_id empty_id;
    file_id_clear(&empty_id);

    fifo = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (fifo) {
        goto unlock; /* found fifo for id; return */
    }

    /* The fifo table is full; create a new entry for the fifo id at the
     * end of the table.
     */

    fifo = append_empty_fifo(self, error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_fifo;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    file_ref_or_set_up(&fifo->base, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_fifo_ref_or_set_up;
    }

unlock:
    unlock_fifotab(self);

    return fifo;

err_fifo_ref_or_set_up:
err_append_empty_fifo:
err_search_by_id:
    unlock_fifotab(self);
    return NULL;
}

size_t
fildes_fifotab_index(struct fildes_fifotab* self, struct fifo* fifo)
{
    return fifo - self->tab;
}
