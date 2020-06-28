/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019-2020  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "seekbuftab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "range.h"

void
fildes_seekbuftab_init(struct fildes_seekbuftab* self,
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
seekbuf_uninit_walk_cb(void* seekbuf, struct picotm_error* error)
{
    seekbuf_uninit(seekbuf);
    return 1;
}

void
fildes_seekbuftab_uninit(struct fildes_seekbuftab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, self->len, sizeof(self->tab[0]),
                     seekbuf_uninit_walk_cb, &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_seekbuftab(struct fildes_seekbuftab* seekbuftab,
                  struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&seekbuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_seekbuftab(struct fildes_seekbuftab* seekbuftab,
                  struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&seekbuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_seekbuftab(struct fildes_seekbuftab* seekbuftab)
{
    do {
        int err = pthread_rwlock_unlock(&seekbuftab->rwlock);
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
static struct seekbuf*
append_empty_seekbuf(struct fildes_seekbuftab* seekbuftab,
                     struct picotm_error* error)
{
    if (seekbuftab->len == picotm_arraylen(seekbuftab->tab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct seekbuf* seekbuf = seekbuftab->tab + seekbuftab->len;

    seekbuf_init(seekbuf, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++seekbuftab->len;

    return seekbuf;
}

/* requires reader lock */
static struct seekbuf*
find_by_id(struct fildes_seekbuftab* seekbuftab, const struct filebuf_id* id)
{
    struct seekbuf *seekbuf_beg = picotm_arraybeg(seekbuftab->tab);
    const struct seekbuf* seekbuf_end = picotm_arrayat(seekbuftab->tab,
                                                       seekbuftab->len);

    while (seekbuf_beg < seekbuf_end) {

        const int cmp = filebuf_ref_if_id(&seekbuf_beg->base, id);
        if (!cmp) {
            return seekbuf_beg;
        }

        ++seekbuf_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct seekbuf*
search_by_id(struct fildes_seekbuftab* seekbuftab, const struct filebuf_id* id,
             int fildes, struct picotm_error* error)
{
    struct seekbuf* seekbuf_beg = picotm_arraybeg(seekbuftab->tab);
    const struct seekbuf* seekbuf_end = picotm_arrayat(seekbuftab->tab,
                                                       seekbuftab->len);

    while (seekbuf_beg < seekbuf_end) {

        const int cmp = filebuf_ref_or_set_up_if_id(&seekbuf_beg->base,
                                                    fildes, id, error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return seekbuf_beg; /* set-up seekbuf structure; return */
        }

        ++seekbuf_beg;
    }

    return NULL;
}

struct seekbuf*
fildes_seekbuftab_ref_fildes(struct fildes_seekbuftab* self, int fildes,
                             struct picotm_error* error)
{
    struct filebuf_id id;
    filebuf_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing seekbuf structure with the given id; iff
     * a new element was not explicitly requested.
     */

    rdlock_seekbuftab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct seekbuf* seekbuf = find_by_id(self, &id);
    if (seekbuf) {
        goto unlock; /* found seekbuf for id; return */
    }

    unlock_seekbuftab(self);

    /* Not found entry; acquire writer lock to create a new entry in
     * the seekbuf table.
     */

    wrlock_seekbuftab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    seekbuf = find_by_id(self, &id);
    if (seekbuf) {
        goto unlock; /* found seekbuf for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, seekbuf structure.
     */

    struct filebuf_id empty_id;
    filebuf_id_clear(&empty_id);

    seekbuf = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (seekbuf) {
        goto unlock; /* found seekbuf for id; return */
    }

    /* The seekbuf table is full; create a new entry for the seekbuf id at the
     * end of the table.
     */

    seekbuf = append_empty_seekbuf(self, error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_seekbuf;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    filebuf_ref_or_set_up(&seekbuf->base, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_seekbuf_ref_or_set_up;
    }

unlock:
    unlock_seekbuftab(self);

    return seekbuf;

err_seekbuf_ref_or_set_up:
err_append_empty_seekbuf:
err_search_by_id:
    unlock_seekbuftab(self);
    return NULL;
}

size_t
fildes_seekbuftab_index(struct fildes_seekbuftab* self,
                        struct seekbuf* seekbuf)
{
    return seekbuf - self->tab;
}
