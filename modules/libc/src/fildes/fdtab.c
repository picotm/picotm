/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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
 */

#include "fdtab.h"
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-rwlock.h>
#include <picotm/picotm-lib-rwstate.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <pthread.h>
#include "fd.h"

static struct fd                fdtab[MAXNUMFD];
static size_t                   fdtab_len = 0;
static pthread_rwlock_t         fdtab_rwlock = PTHREAD_RWLOCK_INITIALIZER;
static struct picotm_rwlock     fdtab_lock;

/* Destructor */

static void fdtab_uninit(void) __attribute__((destructor));

static size_t
fdtab_fd_uninit_walk(void* fd, struct picotm_error* error)
{
    fd_uninit(fd);
    return 1;
}

static void
fdtab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(fdtab, sizeof(fdtab)/sizeof(fdtab[0]), sizeof(fdtab[0]),
                     fdtab_fd_uninit_walk, &error);

    pthread_rwlock_destroy(&fdtab_rwlock);
}

/* End of destructor */

static void
rdlock_fdtab(struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&fdtab_rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_fdtab(struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&fdtab_rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_fdtab(void)
{
    do {
        int err = pthread_rwlock_unlock(&fdtab_rwlock);
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
find_by_id(int fildes, struct picotm_error* error)
{
    if (fdtab_len <= (size_t)fildes) {
        return NULL;
    }

    struct fd* fd = fdtab + fildes;

    fd_ref_or_set_up(fd, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

/* requires writer lock */
static struct fd*
search_by_id(int fildes, struct picotm_error* error)
{
    struct fd* fd_beg = picotm_arrayat(fdtab, fdtab_len);
    const struct fd* fd_end = picotm_arrayat(fdtab, fildes + 1);

    while (fd_beg < fd_end) {

        fd_init(fd_beg, error);
        if (picotm_error_is_set(error)) {
            return NULL;
        }

        ++fdtab_len;
        ++fd_beg;
    }

    struct fd* fd = fdtab + fildes;

    fd_ref_or_set_up(fd, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

struct fd*
fdtab_ref_fildes(int fildes, struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    /* Try to find an existing fd structure with the given file
     * descriptor.
     */

    rdlock_fdtab(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fd* fd = find_by_id(fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_find_by_id;
    } else if (fd) {
        goto unlock; /* found fd for fildes; return */
    }

    unlock_fdtab();

    /* Not found; acquire writer lock to create a new entry
     * in the file-descriptor table.
     */

    wrlock_fdtab(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    fd = search_by_id(fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
        return NULL;
    }

unlock:
    unlock_fdtab();

    return fd;

err_search_by_id:
err_find_by_id:
    unlock_fdtab();
    return NULL;
}

struct fd*
fdtab_get_fd(int fildes)
{
    return fdtab + fildes;
}

void
fdtab_try_rdlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    picotm_rwstate_try_rdlock(lock_state, &fdtab_lock, error);
}

void
fdtab_try_wrlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    picotm_rwstate_try_wrlock(lock_state, &fdtab_lock, error);
}

void
fdtab_unlock(struct picotm_rwstate* lock_state)
{
    picotm_rwstate_unlock(lock_state, &fdtab_lock);
}
