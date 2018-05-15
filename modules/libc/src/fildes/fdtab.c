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

#include "fdtab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <pthread.h>
#include "fd.h"

struct fildes_fdtab {
    struct fd                tab[MAXNUMFD];
    size_t                   len;
    pthread_rwlock_t         rwlock;
    struct picotm_rwlock     lock;
};

#define FILDES_FDTAB_INITIALIZER            \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER,   \
    .lock = PICOTM_RWLOCK_INITIALIZER       \
}

static size_t
fd_uninit_walk_cb(void* fd, struct picotm_error* error)
{
    fd_uninit(fd);
    return 1;
}

static void
fildes_fdtab_uninit(struct fildes_fdtab* self)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->tab, picotm_arraylen(self->tab),
                     sizeof(self->tab[0]), fd_uninit_walk_cb,
                     &error);

    pthread_rwlock_destroy(&self->rwlock);
}

static void
rdlock_fdtab(struct fildes_fdtab* fdtab, struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&fdtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_fdtab(struct fildes_fdtab* fdtab, struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&fdtab->rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_fdtab(struct fildes_fdtab* fdtab)
{
    do {
        int err = pthread_rwlock_unlock(&fdtab->rwlock);
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

/* requires reader lock */
static struct fd*
find_by_id(struct fildes_fdtab* fdtab, int fildes, struct picotm_error* error)
{
    if (fdtab->len <= (size_t)fildes) {
        return NULL;
    }

    struct fd* fd = fdtab->tab + fildes;

    fd_ref_or_set_up(fd, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

/* requires writer lock */
static struct fd*
search_by_id(struct fildes_fdtab* fdtab, int fildes,
             struct picotm_error* error)
{
    struct fd* fd_beg = picotm_arrayat(fdtab->tab, fdtab->len);
    const struct fd* fd_end = picotm_arrayat(fdtab->tab, fildes + 1);

    while (fd_beg < fd_end) {

        fd_init(fd_beg, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }

        ++fdtab->len;
        ++fd_beg;
    }

    struct fd* fd = fdtab->tab + fildes;

    fd_ref_or_set_up(fd, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

struct fd*
fildes_fdtab_ref_fildes(struct fildes_fdtab* self, int fildes,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    /* Try to find an existing fd structure with the given file
     * descriptor.
     */

    rdlock_fdtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fd* fd = find_by_id(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_find_by_id;
    } else if (fd) {
        goto unlock; /* found fd for fildes; return */
    }

    unlock_fdtab(self);

    /* Not found; acquire writer lock to create a new entry
     * in the file-descriptor table.
     */

    wrlock_fdtab(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    fd = search_by_id(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
        return NULL;
    }

unlock:
    unlock_fdtab(self);

    return fd;

err_search_by_id:
err_find_by_id:
    unlock_fdtab(self);
    return NULL;
}

struct fd*
fildes_fdtab_get_fd(struct fildes_fdtab* self, int fildes)
{
    return self->tab + fildes;
}

void
fildes_fdtab_try_rdlock(struct fildes_fdtab* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    picotm_rwstate_try_rdlock(lock_state, &self->lock, error);
}

void
fildes_fdtab_try_wrlock(struct fildes_fdtab* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error)
{
    picotm_rwstate_try_wrlock(lock_state, &self->lock, error);
}

void
fildes_fdtab_unlock(struct fildes_fdtab* self,
                    struct picotm_rwstate* lock_state)
{
    picotm_rwstate_unlock(lock_state, &self->lock);
}

/*
 * Global state
 */

static struct fildes_fdtab fdtab = FILDES_FDTAB_INITIALIZER;

static __attribute__((destructor)) void
fdtab_uninit(void)
{
    fildes_fdtab_uninit(&fdtab);
}

struct fd*
fdtab_ref_fildes(int fildes, struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    return fildes_fdtab_ref_fildes(&fdtab, fildes, lock_state, error);
}

struct fd*
fdtab_get_fd(int fildes)
{
    return fildes_fdtab_get_fd(&fdtab, fildes);
}

void
fdtab_try_rdlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    fildes_fdtab_try_rdlock(&fdtab, lock_state, error);
}

void
fdtab_try_wrlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    fildes_fdtab_try_wrlock(&fdtab, lock_state, error);
}

void
fdtab_unlock(struct picotm_rwstate* lock_state)
{
    fildes_fdtab_unlock(&fdtab, lock_state);
}
