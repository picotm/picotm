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
#include "errcode.h"
#include "fcntlop.h"
#include "mutex.h"
#include "ofdtab.h"

int
fd_init(struct fd *fd)
{
    assert(fd);

    int err;

    if ((err = mutex_init(&fd->lock, NULL)) < 0) {
        return err;
    }

    atomic_init(&fd->ref, 0);

    fd->state = FD_ST_UNUSED;
    fd->ofd = -1;

    atomic_init(&fd->ver, 0);

    return 0;
}

void
fd_uninit(struct fd *fd)
{
    mutex_uninit(&fd->lock);
}

void
fd_lock(struct fd *fd)
{
    assert(fd);

    mutex_lock(&fd->lock);
}

void
fd_unlock(struct fd *fd)
{
    assert(fd);

    mutex_unlock(&fd->lock);
}

int
fd_is_open_nl(const struct fd *fd)
{
	assert(fd);

	return fd->state == FD_ST_INUSE;
}

int
fd_validate(struct fd* fd, unsigned long ver, struct picotm_error* error)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    unsigned long myver = atomic_load(&fd->ver);

    if (ver < myver) {
        picotm_error_set_conflicting(error, NULL);
        return ERR_CONFLICT;
    }

    return 0;
}

static int
fd_setup_ofd(struct fd *fd, int fildes, unsigned long flags)
{
    int ofd = ofdtab_ref_ofd(fildes, flags);

    if (ofd < 0) {
        return ofd;
    }

    fd->ofd = ofd;

    return 0;
}

int
fd_ref_state(struct fd *fd, int fildes, unsigned long flags, int *ofd, unsigned long *version)
{
    assert(fd);

    int err = 0;

    fd_lock(fd);

    switch (fd->state) {
        case FD_ST_CLOSING:
            /* fd is about to be closed; abort here */
            err = ERR_CONFLICT;
            break;
        case FD_ST_UNUSED:
            /* setup new fd data structure */
            err = fd_setup_ofd(fd, fildes, flags);

            if (__builtin_expect(!err, 1)) {
                fd->state = FD_ST_INUSE;
                atomic_store(&fd->ref, 1); /* Exactly one reference */
            }
            break;
        case FD_ST_INUSE:
            /* simply return version and increment reference counter */
            if (flags&OFD_FL_WANTNEW) {
            	err = ERR_CONFLICT;
            } else {
                atomic_fetch_add(&fd->ref, 1);
            }
            break;
        default:
            abort();
    }

    if (!err && fd_is_open_nl(fd)) {
        if (ofd) {
            *ofd = fd_get_ofd_nl(fd);
        }
        if (version) {
            *version = fd_get_version_nl(fd);
        }
    }

    fd_unlock(fd);

    return err;
}

int
fd_ref(struct fd *fd, int fildes, unsigned long flags)
{
    return fd_ref_state(fd, fildes, flags, NULL, NULL);
}

static void
fd_cleanup(struct fd *fd)
{
    assert(fd);

    ofd_unref(ofdtab+fd->ofd);

    fd->state = FD_ST_UNUSED;
    fd->ofd = -1;
}

void
fd_unref(struct fd *fd, int fildes)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    fd_lock(fd);

    switch (fd->state) {
        case FD_ST_CLOSING: {
            unsigned long oldref = atomic_fetch_sub(&fd->ref, 1);
            if (oldref == 1) {
                fd_cleanup(fd);

				/* finally close fildes, if we released the last reference */
				if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
                    abort(); /* FIXME: Raise error flag */
				}
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

bool
fd_is_valid(struct fd* fd, unsigned long version)
{
    return atomic_load(&fd->ver) <= version;
}

int
fd_setfd(struct fd* fd, int fildes, int arg, struct picotm_error* error)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    int res = TEMP_FAILURE_RETRY(fcntl(fildes, F_SETFD, arg));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
fd_getfd(struct fd* fd, int fildes, struct picotm_error* error)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < (ssize_t)(sizeof(ofdtab)/sizeof(ofdtab[0])));

    int res = TEMP_FAILURE_RETRY(fcntl(fildes, F_GETFD));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}
