/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "errcode.h"
#include "types.h"
#include "seekop.h"
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
#include "fd.h"
#include "fdtab.h"
#include "ofdtx.h"
#include "fdtx.h"
#include "comfd.h"

int
com_fd_exec_close(struct com_fd *data, int fildes, int isnoundo)
{
    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Close */

    int cookie = -1;

    err = fdtx_close_exec(fdtx, fildes, &cookie, isnoundo);

    if (err < 0) {
        return err;
    }

    /* Inject event */
    if ((cookie >= 0)
         && (com_fd_inject(data, ACTION_CLOSE, fildes, -1) < 0)) {
        return -1;
    }

    return 0;
}

int
com_fd_apply_close(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        err = fdtx_close_apply(fdtx, event->fildes, event->cookie) < 0 ? -1 : 0;
        --n;
        ++event;
    }

    return err;
}

int
com_fd_undo_close(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    return fdtx_close_undo(fdtx, fildes, cookie) < 0 ? -1 : 0;
}

