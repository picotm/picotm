/* Copyright (C) 2008-2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
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

