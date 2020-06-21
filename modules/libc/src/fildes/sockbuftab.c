/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "sockbuftab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "range.h"

void
fildes_sockbuftab_init(struct fildes_sockbuftab* self,
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
sockbuf_uninit_walk_cb(void* sockbuf, struct picotm_error* error)
{
    sockbuf_uninit(sockbuf);
    return 1;
}

void
fildes_sockbuftab_uninit(struct fildes_sockbuftab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, self->len, sizeof(self->tab[0]),
                     sockbuf_uninit_walk_cb, &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_sockbuftab(struct fildes_sockbuftab* sockbuftab,
                  struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&sockbuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_sockbuftab(struct fildes_sockbuftab* sockbuftab,
                  struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&sockbuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_sockbuftab(struct fildes_sockbuftab* sockbuftab)
{
    do {
        int err = pthread_rwlock_unlock(&sockbuftab->rwlock);
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
static struct sockbuf*
append_empty_sockbuf(struct fildes_sockbuftab* sockbuftab,
                     struct picotm_error* error)
{
    if (sockbuftab->len == picotm_arraylen(sockbuftab->tab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct sockbuf* sockbuf = sockbuftab->tab + sockbuftab->len;

    sockbuf_init(sockbuf, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++sockbuftab->len;

    return sockbuf;
}

/* requires reader lock */
static struct sockbuf*
find_by_id(struct fildes_sockbuftab* sockbuftab, const struct filebuf_id* id)
{
    struct sockbuf *sockbuf_beg = picotm_arraybeg(sockbuftab->tab);
    const struct sockbuf* sockbuf_end = picotm_arrayat(sockbuftab->tab,
                                                       sockbuftab->len);

    while (sockbuf_beg < sockbuf_end) {

        const int cmp = sockbuf_ref_if_id(sockbuf_beg, id);
        if (!cmp) {
            return sockbuf_beg;
        }

        ++sockbuf_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct sockbuf*
search_by_id(struct fildes_sockbuftab* sockbuftab, const struct filebuf_id* id,
             int fildes, struct picotm_error* error)
{
    struct sockbuf* sockbuf_beg = picotm_arraybeg(sockbuftab->tab);
    const struct sockbuf* sockbuf_end = picotm_arrayat(sockbuftab->tab,
                                                       sockbuftab->len);

    while (sockbuf_beg < sockbuf_end) {

        const int cmp = sockbuf_ref_or_set_up_if_id(sockbuf_beg, fildes,
                                                    id, error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return sockbuf_beg; /* set-up sockbuf structure; return */
        }

        ++sockbuf_beg;
    }

    return NULL;
}

struct sockbuf*
fildes_sockbuftab_ref_fildes(struct fildes_sockbuftab* self, int fildes,
                             struct picotm_error* error)
{
    struct filebuf_id id;
    filebuf_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing sockbuf structure with the given id; iff
     * a new element was not explicitly requested.
     */

    rdlock_sockbuftab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct sockbuf* sockbuf = find_by_id(self, &id);
    if (sockbuf) {
        goto unlock; /* found sockbuf for id; return */
    }

    unlock_sockbuftab(self);

    /* Not found entry; acquire writer lock to create a new entry in
     * the sockbuf table.
     */

    wrlock_sockbuftab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    sockbuf = find_by_id(self, &id);
    if (sockbuf) {
        goto unlock; /* found sockbuf for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, sockbuf structure.
     */

    struct filebuf_id empty_id;
    filebuf_id_clear(&empty_id);

    sockbuf = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (sockbuf) {
        goto unlock; /* found sockbuf for id; return */
    }

    /* The sockbuf table is full; create a new entry for the sockbuf id at the
     * end of the table.
     */

    sockbuf = append_empty_sockbuf(self, error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_sockbuf;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    sockbuf_ref_or_set_up(sockbuf, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_sockbuf_ref_or_set_up;
    }

unlock:
    unlock_sockbuftab(self);

    return sockbuf;

err_sockbuf_ref_or_set_up:
err_append_empty_sockbuf:
err_search_by_id:
    unlock_sockbuftab(self);
    return NULL;
}

size_t
fildes_sockbuftab_index(struct fildes_sockbuftab* self,
                        struct sockbuf* sockbuf)
{
    return sockbuf - self->tab;
}