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

#include "pipebuftab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "range.h"

void
fildes_pipebuftab_init(struct fildes_pipebuftab* self,
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
pipebuf_uninit_walk_cb(void* pipebuf, struct picotm_error* error)
{
    pipebuf_uninit(pipebuf);
    return 1;
}

void
fildes_pipebuftab_uninit(struct fildes_pipebuftab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, self->len, sizeof(self->tab[0]),
                     pipebuf_uninit_walk_cb, &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_pipebuftab(struct fildes_pipebuftab* pipebuftab,
                  struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&pipebuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_pipebuftab(struct fildes_pipebuftab* pipebuftab,
                  struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&pipebuftab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_pipebuftab(struct fildes_pipebuftab* pipebuftab)
{
    do {
        int err = pthread_rwlock_unlock(&pipebuftab->rwlock);
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
static struct pipebuf*
append_empty_pipebuf(struct fildes_pipebuftab* pipebuftab,
                     struct picotm_error* error)
{
    if (pipebuftab->len == picotm_arraylen(pipebuftab->tab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct pipebuf* pipebuf = pipebuftab->tab + pipebuftab->len;

    pipebuf_init(pipebuf, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++pipebuftab->len;

    return pipebuf;
}

/* requires reader lock */
static struct pipebuf*
find_by_id(struct fildes_pipebuftab* pipebuftab, const struct filebuf_id* id)
{
    struct pipebuf *pipebuf_beg = picotm_arraybeg(pipebuftab->tab);
    const struct pipebuf* pipebuf_end = picotm_arrayat(pipebuftab->tab,
                                                       pipebuftab->len);

    while (pipebuf_beg < pipebuf_end) {

        const int cmp = filebuf_ref_if_id(&pipebuf_beg->base, id);
        if (!cmp) {
            return pipebuf_beg;
        }

        ++pipebuf_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct pipebuf*
search_by_id(struct fildes_pipebuftab* pipebuftab, const struct filebuf_id* id,
             int fildes, struct picotm_error* error)
{
    struct pipebuf* pipebuf_beg = picotm_arraybeg(pipebuftab->tab);
    const struct pipebuf* pipebuf_end = picotm_arrayat(pipebuftab->tab,
                                                       pipebuftab->len);

    while (pipebuf_beg < pipebuf_end) {

        const int cmp = filebuf_ref_or_set_up_if_id(&pipebuf_beg->base,
                                                    fildes, id, error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return pipebuf_beg; /* set-up pipebuf structure; return */
        }

        ++pipebuf_beg;
    }

    return NULL;
}

struct pipebuf*
fildes_pipebuftab_ref_fildes(struct fildes_pipebuftab* self, int fildes,
                             struct picotm_error* error)
{
    struct filebuf_id id;
    filebuf_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing pipebuf structure with the given id; iff
     * a new element was not explicitly requested.
     */

    rdlock_pipebuftab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct pipebuf* pipebuf = find_by_id(self, &id);
    if (pipebuf) {
        goto unlock; /* found pipebuf for id; return */
    }

    unlock_pipebuftab(self);

    /* Not found entry; acquire writer lock to create a new entry in
     * the pipebuf table.
     */

    wrlock_pipebuftab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    pipebuf = find_by_id(self, &id);
    if (pipebuf) {
        goto unlock; /* found pipebuf for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, pipebuf structure.
     */

    struct filebuf_id empty_id;
    filebuf_id_clear(&empty_id);

    pipebuf = search_by_id(self, &empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (pipebuf) {
        goto unlock; /* found pipebuf for id; return */
    }

    /* The pipebuf table is full; create a new entry for the pipebuf id at the
     * end of the table.
     */

    pipebuf = append_empty_pipebuf(self, error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_pipebuf;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    filebuf_ref_or_set_up(&pipebuf->base, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_pipebuf_ref_or_set_up;
    }

unlock:
    unlock_pipebuftab(self);

    return pipebuf;

err_pipebuf_ref_or_set_up:
err_append_empty_pipebuf:
err_search_by_id:
    unlock_pipebuftab(self);
    return NULL;
}

size_t
fildes_pipebuftab_index(struct fildes_pipebuftab* self,
                        struct pipebuf* pipebuf)
{
    return pipebuf - self->tab;
}
