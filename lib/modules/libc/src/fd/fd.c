/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "errcode.h"
#include "types.h"
#include "range.h"
#include "mutex.h"
#include "rwlock.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtab.h"
#include "fd.h"

int
fd_init(struct fd *fd)
{
    assert(fd);

    int err;

    if ((err = mutex_init(&fd->lock, NULL)) < 0) {
        return err;
    }

    if ((err = counter_init(&fd->ref)) < 0) {
        mutex_uninit(&fd->lock);
        return err;
    }

    fd->state = FD_ST_UNUSED;
    fd->ofd = -1;

    if ((err = counter_init(&fd->ver)) < 0) {
        counter_uninit(&fd->ref);
        mutex_uninit(&fd->lock);
        return err;
    }

    return 0;
}

void
fd_uninit(struct fd *fd)
{
    counter_uninit(&fd->ver);
    counter_uninit(&fd->ref);
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
fd_validate(struct fd *fd, count_type ver)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < sizeof(ofdtab)/sizeof(ofdtab[0]));

    int res;

    count_type myver = counter_get(&fd->ver);

    if (ver < myver) {
        res = ERR_CONFLICT;
    } else {
        res = 0;
    }

    return res;
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
fd_ref_state(struct fd *fd, int fildes, unsigned long flags, int *ofd, count_type *version)
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
                counter_set(&fd->ref, 1); /* Exactly one reference */
            }
            break;
        case FD_ST_INUSE:
            /* simply return version and increment reference counter */
            if (flags&OFD_FL_WANTNEW) {
            	err = ERR_CONFLICT;
            } else {
                counter_inc(&fd->ref);
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
    assert(fd->ofd < sizeof(ofdtab)/sizeof(ofdtab[0]));

    fd_lock(fd);

    switch (fd->state) {
        case FD_ST_CLOSING:
            if (!counter_dec(&fd->ref)) {
                fd_cleanup(fd);

				/* finally close fildes, if we released the last reference */
				if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
                    abort(); /* FIXME: Raise error flag */
				}
            }
            break;
        case FD_ST_INUSE:
            if (!counter_dec(&fd->ref)) {
                fd_cleanup(fd);
            }
            break;
        case FD_ST_UNUSED:
        default:
            abort(); /* should never happen */
    }

    fd_unlock(fd);
}

count_type
fd_get_version_nl(struct fd *fd)
{
    assert(fd);

    count_type res;

    switch (fd->state) {
        case FD_ST_INUSE:
            res = counter_get(&fd->ver);
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

/* fcntl
 */

int
fd_fcntl_exec(struct fd *fd, int fildes, int cmd,
                                         union com_fd_fcntl_arg *arg,
                                         union com_fd_fcntl_arg *oldarg,
                                         count_type ver, int noundo)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < sizeof(ofdtab)/sizeof(ofdtab[0]));

    int res;

    if (ver < counter_get(&fd->ver)) {
        res = ERR_CONFLICT;
    } else {

        switch (cmd) {
            case F_SETFD:
                if ( !noundo ) {
                    return ERR_NOUNDO;
                }
                res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg0));
                break;
            case F_GETFD:
                arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
                if (arg->arg0 < 0) {
                    res = ERR_SYSTEM;
                }
                res = 0;
                break;
            default:
                res = ERR_DOMAIN;
                break;
        }
    }

    return res;
}

int
fd_fcntl_apply(struct fd *fd, int fildes, int cmd,
                              union com_fd_fcntl_arg *arg, int ccmode)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < sizeof(ofdtab)/sizeof(ofdtab[0]));

    int res = 0;

    switch (cmd) {
        case F_SETFD:
            res = fcntl(fildes, cmd, arg->arg0);
            /* Fall through */
        case F_GETFD:
            break;
        default:
            res = ERR_DOMAIN; /* Caller should try other domain, e.g OFD. */
            break;
    }

    return res;
}

int
fd_fcntl_undo(struct fd *fd, int fildes, int cmd,
                             union com_fd_fcntl_arg *oldarg, int ccmode)
{
    assert(fd);
    assert(fd->ofd >= 0);
    assert(fd->ofd < sizeof(ofdtab)/sizeof(ofdtab[0]));

    int res = 0;

    switch (cmd) {
        case F_SETFD:
            res = fcntl(fildes, cmd, oldarg->arg0);
            break;
        case F_GETFD:
            break;
        default:
            res = ERR_DOMAIN; /* Caller should try other domain, e.g OFD. */
            break;
    }

    return res;
}

