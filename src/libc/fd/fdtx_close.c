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

/*
 * Exec
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
        fdtx->cc_mode = SYSTX_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((fdtx->cc_mode == SYSTX_LIBC_CC_MODE_NOUNDO)
            || !close_exec[fdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return close_exec[fdtx->cc_mode](fdtx, fildes, cookie);
}

/*
 * Apply
 */

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

/*
 * Undo
 */

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

