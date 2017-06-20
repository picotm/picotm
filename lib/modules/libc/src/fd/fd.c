/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fd.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm-error.h>
#include <stdlib.h>
#include <unistd.h>
#include "fcntlop.h"

void
fd_init(struct fd *fd, struct picotm_error* error)
{
    assert(fd);

    int err = pthread_mutex_init(&fd->lock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    picotm_ref_init(&fd->ref, 0);

    fd->fildes = -1;
    fd->state = FD_ST_UNUSED;

    atomic_init(&fd->ver, 0);
}

void
fd_uninit(struct fd *fd)
{
    int err = pthread_mutex_destroy(&fd->lock);
    if (err) {
        abort();
    }
}

void
fd_lock(struct fd *fd)
{
    assert(fd);

    int err = pthread_mutex_lock(&fd->lock);
    if (err) {
        abort();
    }
}

void
fd_unlock(struct fd *fd)
{
    assert(fd);

    int err = pthread_mutex_unlock(&fd->lock);
    if (err) {
        abort();
    }
}

int
fd_is_open_nl(const struct fd *fd)
{
	assert(fd);

	return fd->state == FD_ST_INUSE;
}

void
fd_validate(struct fd* fd, unsigned long ver, struct picotm_error* error)
{
    assert(fd);

    unsigned long myver = atomic_load(&fd->ver);

    if (ver < myver) {
        picotm_error_set_conflicting(error, NULL);
        return;
    }
}

static bool
incr_ref_count(struct fd *fd, struct picotm_error* error)
{
    bool first_ref;

    switch (fd->state) {
        case FD_ST_UNUSED:
            /* fall through */
        case FD_ST_INUSE:
            first_ref = picotm_ref_up(&fd->ref);
            break;
        case FD_ST_CLOSING:
            /* fd is about to be closed; abort here */
            picotm_error_set_conflicting(error, NULL);
            return false;
        default:
            abort();
    }

    return first_ref;
}

void
fd_ref(struct fd *fd, struct picotm_error* error)
{
    incr_ref_count(fd, error);
}

void
fd_ref_state(struct fd *fd, unsigned long *version,
             struct picotm_error* error)
{
    assert(fd);

    fd_ref(fd, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    fd_lock(fd);

    if (fd_is_open_nl(fd)) {
        if (version) {
            *version = fd_get_version_nl(fd);
        }
    }

    fd_unlock(fd);
}

void
fd_ref_or_set_up(struct fd *fd, int fildes, bool want_new,
                 struct picotm_error* error)
{
    fd_lock(fd);

    bool first_ref = incr_ref_count(fd, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    if (!first_ref && !want_new) {
        /* we got a set-up instance; signal success */
        goto unlock;
    }

    if (!first_ref && want_new) {
        picotm_error_set_conflicting(error, NULL);
        goto err_want_new;
    }

    /* set up instance */
    fd->fildes = fildes;
    fd->state = FD_ST_INUSE;

unlock:
    fd_unlock(fd);

    return;

err_want_new:
    fd_unlock(fd);
    fd_unref(fd);
}

static void
fd_cleanup(struct fd *fd)
{
    assert(fd);

    fd->fildes = -1;
    fd->state = FD_ST_UNUSED;
}

void
fd_unref(struct fd *fd)
{
    assert(fd);

    fd_lock(fd);

    bool final_ref = picotm_ref_down(&fd->ref);
    if (!final_ref) {
        goto unlock;
    }

    switch (fd->state) {
        case FD_ST_UNUSED:
        case FD_ST_INUSE: {
            fd_cleanup(fd);
            break;
        }
        case FD_ST_CLOSING: {
            /* close fildes if we released the final reference */
            int res = TEMP_FAILURE_RETRY(close(fd->fildes));
            if (res < 0) {
                abort(); /* FIXME: Raise error flag */
            }

            fd_cleanup(fd);
            break;
        }
        default:
            abort(); /* should never happen */
    }

unlock:
    fd_unlock(fd);
}

unsigned long
fd_get_version_nl(struct fd *fd)
{
    assert(fd);

    unsigned long res;

    switch (fd->state) {
        case FD_ST_INUSE:
            res = atomic_load(&fd->ver);
            break;
        case FD_ST_CLOSING:
        case FD_ST_UNUSED:
        default:
            abort();
    }

    return res;
}

void
fd_close(struct fd *fd)
{
    assert(fd);

    fd->state = FD_ST_CLOSING;
}

void
fd_dump(const struct fd *fd)
{
    return;
}

int
fd_setfd(struct fd* fd, int arg, struct picotm_error* error)
{
    assert(fd);

    int res = TEMP_FAILURE_RETRY(fcntl(fd->fildes, F_SETFD, arg));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
fd_getfd(struct fd* fd, struct picotm_error* error)
{
    assert(fd);

    int res = TEMP_FAILURE_RETRY(fcntl(fd->fildes, F_GETFD));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}
