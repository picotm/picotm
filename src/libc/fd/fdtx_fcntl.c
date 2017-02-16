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
#include <tanger-stm-internal-errcode.h>
#include "tanger-stm-ext-actions.h"
#include "types.h"
#include "range.h"
#include "mutex.h"
#include "table.h"
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

/*
 * Apply
 */

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
                                              fdtx->ccmode);
}

/*
 * Undo
 */

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
                                                fdtx->ccmode);
    fd_unlock(fdtab+fdtx->fildes);

    return res;
}

