/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fdtx.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "errcode.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "fdtab.h"
#include "ofd.h"

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

/*
 * close()
 */

static int
fdtx_close_exec_noundo(struct fdtx *fdtx, int fildes, int *cookie)
{
    return 0;
}

static int
fdtx_close_exec_ts(struct fdtx *fdtx, int fildes, int *cookie)
{
    return 0;
}

int
fdtx_close_exec(struct fdtx *fdtx, int fildes, int *cookie, int noundo)
{
    static int (* const close_exec[2])(struct fdtx*, int, int*) = {
        fdtx_close_exec_noundo, fdtx_close_exec_ts};

    assert(fdtx->cc_mode < sizeof(close_exec)/sizeof(close_exec[0]));

    if (noundo) {
        /* TX irrevokable */
        fdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((fdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !close_exec[fdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return close_exec[fdtx->cc_mode](fdtx, fildes, cookie);
}

static int
fdtx_close_apply_noundo(struct fdtx *fdtx, int fildes, int cookie)
{
    fd_close(fdtab+fildes);
    return 0;
}

static int
fdtx_close_apply_ts(struct fdtx *fdtx, int fildes, int cookie)
{
    /* Global data structure 'fdtab' is locked during apply */
    fd_close(fdtab+fildes);
    return 0;
}

int
fdtx_close_apply(struct fdtx *fdtx, int fildes, int cookie)
{
    static int (* const close_apply[2])(struct fdtx*, int, int) = {
        fdtx_close_apply_noundo, fdtx_close_apply_ts};

    assert(fdtx->cc_mode < sizeof(close_apply)/sizeof(close_apply[0]));

    return close_apply[fdtx->cc_mode](fdtx, fildes, cookie);
}

static int
fdtx_close_undo_ts(struct fdtx *fdtx, int fildes, int cookie)
{
    return 0;
}

int
fdtx_close_undo(struct fdtx *fdtx, int fildes, int cookie)
{
    static int (* const close_undo[2])(struct fdtx*, int, int) = {
        NULL, fdtx_close_undo_ts};

    assert(fdtx->cc_mode < sizeof(close_undo)/sizeof(close_undo[0]));
    assert(close_undo[fdtx->cc_mode]);

    return close_undo[fdtx->cc_mode](fdtx, fildes, cookie);
}

/*
 * fcntl()
 */

int
fdtx_fcntl_exec(struct fdtx *fdtx, int cmd, union com_fd_fcntl_arg *arg,
                                   int *cookie, int noundo)
{
    assert(fdtx);
    assert(fdtx->fildes >= 0);
    assert(fdtx->fildes < sizeof(fdtab)/sizeof(fdtab[0]));

    union com_fd_fcntl_arg oldvalue;

    fd_lock(fdtab+fdtx->fildes);
    int res = fd_fcntl_exec(fdtab+fdtx->fildes,
                                  fdtx->fildes, cmd, arg, &oldvalue,
                                  fdtx->fdver, noundo);
    fd_unlock(fdtab+fdtx->fildes);

    if (res < 0) {
        return res;
    }

    /* register fcntl */

    if (cookie) {
        *cookie = fcntloptab_append(&fdtx->fcntltab,
                                    &fdtx->fcntltablen, cmd, arg, &oldvalue);

        if (*cookie < 0) {
            abort();
        }
    }

	fdtx->flags |= FDTX_FL_LOCALSTATE;

    return res;
}

int
fdtx_fcntl_apply(struct fdtx *fdtx, int cookie)
{
    assert(fdtx);
    assert(fdtx->fildes >= 0);
    assert(fdtx->fildes < sizeof(fdtab)/sizeof(fdtab[0]));
    assert(cookie < fdtx->fcntltablen);

    return fd_fcntl_apply(fdtab+fdtx->fildes,
                                fdtx->fildes, fdtx->fcntltab[cookie].command,
                                             &fdtx->fcntltab[cookie].value,
                                              fdtx->cc_mode);
}

int
fdtx_fcntl_undo(struct fdtx *fdtx, int cookie)
{
    assert(fdtx);
    assert(fdtx->fildes >= 0);
    assert(fdtx->fildes < sizeof(fdtab)/sizeof(fdtab[0]));
    assert(cookie < fdtx->fcntltablen);

    fd_lock(fdtab+fdtx->fildes);
    int res = fd_fcntl_undo(fdtab+fdtx->fildes,
                                  fdtx->fildes, fdtx->fcntltab[cookie].command,
                                               &fdtx->fcntltab[cookie].oldvalue,
                                                fdtx->cc_mode);
    fd_unlock(fdtab+fdtx->fildes);

    return res;
}

