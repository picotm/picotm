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
#include "ofdtab.h"

void
fd_init(struct fd *fd, struct picotm_error* error)
{
    assert(fd);

    int err = pthread_mutex_init(&fd->lock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    atomic_init(&fd->ref, 0);

    fd->fildes = -1;
    fd->state = FD_ST_UNUSED;
    fd->ofd = -1;

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
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    unsigned long myver = atomic_load(&fd->ver);

    if (ver < myver) {
        picotm_error_set_conflicting(error, NULL);
        return;
    }
}

static void
fd_setup_ofd(struct fd *fd, int fildes, unsigned long flags,
             struct picotm_error* error)
{
    int ofd = ofdtab_ref_ofd(fildes, flags, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    fd->ofd = ofd;
}

void
fd_ref_state(struct fd *fd, int fildes, unsigned long flags, int *ofd,
             unsigned long *version, struct picotm_error* error)
{
    assert(fd);

    fd_lock(fd);

    switch (fd->state) {
        case FD_ST_CLOSING:
            /* fd is about to be closed; abort here */
            picotm_error_set_conflicting(error, NULL);
            goto unlock;
            break;
        case FD_ST_UNUSED:
            /* setup new fd data structure */
            fd_setup_ofd(fd, fildes, flags, error);
            if (picotm_error_is_set(error)) {
                goto unlock;
            }

            fd->fildes = fildes;
            fd->state = FD_ST_INUSE;
            atomic_store(&fd->ref, 1); /* Exactly one reference */
            break;
        case FD_ST_INUSE:
            /* simply return version and increment reference counter */
            if (flags & OFD_FL_WANTNEW) {
                picotm_error_set_conflicting(error, NULL);
                goto unlock;
            }
            atomic_fetch_add(&fd->ref, 1);
            break;
        default:
            abort();
    }

    if (fd_is_open_nl(fd)) {
        if (ofd) {
            *ofd = fd_get_ofd_nl(fd);
        }
        if (version) {
            *version = fd_get_version_nl(fd);
        }
    }

unlock:
    fd_unlock(fd);
}

void
fd_ref(struct fd *fd, int fildes, unsigned long flags,
       struct picotm_error* error)
{
    fd_ref_state(fd, fildes, flags, NULL, NULL, error);
}

static void
fd_cleanup(struct fd *fd)
{
    assert(fd);

    ofd_unref(ofdtab+fd->ofd);

    fd->fildes = -1;
    fd->state = FD_ST_UNUSED;
    fd->ofd = -1;
}

void
fd_unref(struct fd *fd)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    fd_lock(fd);

    switch (fd->state) {
        case FD_ST_CLOSING: {
            unsigned long oldref = atomic_fetch_sub(&fd->ref, 1);
            if (oldref == 1) {
                /* finally close fildes, if we released the last reference */
                int res = TEMP_FAILURE_RETRY(close(fd->fildes));
                if (res < 0) {
                    abort(); /* FIXME: Raise error flag */
                }

                fd_cleanup(fd);
            }
            break;
        }
        case FD_ST_INUSE: {
            unsigned long oldref = atomic_fetch_sub(&fd->ref, 1);
            if (oldref == 1) {
                fd_cleanup(fd);
            }
            break;
        }
        case FD_ST_UNUSED:
        default:
            abort(); /* should never happen */
    }

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

int
fd_get_ofd_nl(struct fd *fd)
{
    assert(fd);

    return fd->ofd;
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
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

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
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    int res = TEMP_FAILURE_RETRY(fcntl(fd->fildes, F_GETFD));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}
