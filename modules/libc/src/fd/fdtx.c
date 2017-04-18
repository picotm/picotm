/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
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
#include "ioop.h"
#include "iooptab.h"
#include "seekop.h"
#include "seekoptab.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "ofdid.h"
#include "ofd.h"
#include "fd.h"
#include "fdtab.h"
#include "fdtx.h"

int
fdtx_init(struct fdtx *fdtx)
{
    assert(fdtx);

    fdtx->fildes = -1;
    fdtx->ofd = -1;
	fdtx->flags = 0;
	fdtx->cc_mode = PICOTM_LIBC_CC_MODE_TS;

    fdtx->fcntltab = NULL;
    fdtx->fcntltablen = 0;

    fdtx->fdver = 0;

    return 0;
}

void
fdtx_uninit(struct fdtx *fdtx)
{
    assert(fdtx);
}

int
fdtx_validate(struct fdtx *fdtx)
{
	int res;

    assert(fdtx);

    if (!fdtx_holds_ref(fdtx)) {
        return 0;
    }

	/* file descriptor is still open; previously locked */
	if (!fd_is_open_nl(fdtab+fdtx->fildes)) {
		return ERR_CONFLICT;
	}

	/* fastpath: no dependencies to other domains */
	if (!(fdtx->flags&FDTX_FL_LOCALSTATE)) {
		return 0;
	}

	/* validate version of file descriptor */
    if ((res = fd_validate(fdtab+fdtx->fildes, fdtx->fdver)) < 0) {
        return res;
    }

    return 0;
}

int
fdtx_updatecc(struct fdtx *fdtx)
{
    assert(fdtx);
    assert(fdtx_holds_ref(fdtx));

    return 0;
}

int
fdtx_clearcc(struct fdtx *fdtx)
{
    assert(fdtx);
    assert(fdtx_holds_ref(fdtx));

    return 0;
}

enum error_code
fdtx_ref_or_validate(struct fdtx *fdtx, int fildes, unsigned long flags)
{
    assert(fdtx);
    assert(fildes >= 0);
    assert(fildes < sizeof(fdtab)/sizeof(fdtab[0]));

    count_type fdver;

    struct fd *fd = fdtab+fildes;

    if (fdtx_holds_ref(fdtx)) {

        /* Validate reference or return error if fd has been closed */

        fd_lock(fd);

        if (fd_is_open_nl(fd)) {
            fdver = fd_get_version_nl(fd);
        } else {
            fd_unlock(fd);
            return ERR_CONFLICT;
        }

        fd_unlock(fd);

        if (fdver > fdtx->fdver) {
            return ERR_CONFLICT;
        }

    } else {

        /* Aquire reference if possible */

        int ofd;

        int err = fd_ref_state(fd, fildes, flags, &ofd, &fdver);

        if (err) {
            return err;
        }

        fdtx->fildes = fildes;
        fdtx->ofd = ofd;
        fdtx->fdver = fdver;
    	fdtx->flags = flags&OFD_FL_WANTNEW ? FDTX_FL_LOCALSTATE : 0;
    }

    return 0;
}


enum error_code
fdtx_ref(struct fdtx *fdtx, int fildes, unsigned long flags)
{
    assert(fdtx);
    assert(fildes >= 0);
    assert(fildes < sizeof(fdtab)/sizeof(fdtab[0]));

    if (fdtx_holds_ref(fdtx)) {
        return 0;
    }

    struct fd *fd = fdtab+fildes;

    /* aquire reference if possible */

    count_type fdver;
    int ofd;

    int err = fd_ref_state(fd, fildes, flags, &ofd, &fdver);

    if (err) {
        return err;
    }

    fdtx->fildes = fildes;
    fdtx->ofd = ofd;
    fdtx->fdver = fdver;
    fdtx->flags = flags&OFD_FL_WANTNEW ? FDTX_FL_LOCALSTATE : 0;

    return 0;
}

void
fdtx_unref(struct fdtx *fdtx)
{
    assert(fdtx);

    if (!fdtx_holds_ref(fdtx)) {
        return;
    }

    fd_unref(fdtab+fdtx->fildes, fdtx->fildes);

    fdtx->flags = 0;
    fdtx->fildes = -1;
}

int
fdtx_holds_ref(const struct fdtx *fdtx)
{
    assert(fdtx);

    return (fdtx->fildes >= 0) &&
           (fdtx->fildes < sizeof(fdtab)/sizeof(fdtab[0]));
}

int
fdtx_pre_commit(struct fdtx *fdtx)
{
    assert(fdtx);

    /* file descriptor has local changes */

    if (fdtx->flags&FDTX_FL_LOCALSTATE) {
        fd_lock(fdtab+fdtx->fildes);
    }

    return 0;
}

int
fdtx_post_commit(struct fdtx *fdtx)
{
    assert(fdtx);

    /* unlock file descriptor at the end of commit */

    if (fdtx->flags&FDTX_FL_LOCALSTATE) {
        fd_unlock(fdtab+fdtx->fildes);
    }

    return 0;
}

void
fdtx_signal_close(struct fdtx *fdtx)
{
    assert(fdtx);

    fd_close(fdtab+fdtx->fildes);
}

void
fdtx_dump(const struct fdtx *fdtx)
{
    fprintf(stderr, "%p: %d %p %zu\n", (void*)fdtx,
                                              fdtx->fildes,
                                       (void*)fdtx->fcntltab,
                                              fdtx->fcntltablen);
}

