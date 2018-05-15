/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include "fifotab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include <pthread.h>
#include "fifo.h"
#include "range.h"

struct fildes_fifotab {
    struct fifo      tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_FIFOTAB_INITIALIZER          \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

static size_t
fifo_uninit_walk_cb(void* fifo, struct picotm_error* error)
{
    fifo_uninit(fifo);
    return 1;
}

static void
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

        const int cmp = fifo_cmp_and_ref(fifo_beg, id);
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

        const int cmp = fifo_cmp_and_ref_or_set_up(fifo_beg, id, fildes,
                                                   error);
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

static struct fifo*
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
    fifo_ref_or_set_up(fifo, fildes, error);
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

static size_t
fildes_fifotab_index(struct fildes_fifotab* self, struct fifo* fifo)
{
    return fifo - self->tab;
}

/*
 * Global state
 */

static struct fildes_fifotab fifotab = FILDES_FIFOTAB_INITIALIZER;

static __attribute__((destructor)) void
fifotab_uninit(void)
{
    fildes_fifotab_uninit(&fifotab);
}

struct fifo*
fifotab_ref_fildes(int fildes, struct picotm_error* error)
{
    return fildes_fifotab_ref_fildes(&fifotab, fildes, error);
}

size_t
fifotab_index(struct fifo* fifo)
{
    return fildes_fifotab_index(&fifotab, fifo);
}
