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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
#include "types.h"
#include "rwlock.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "seekop.h"
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtx.h"
#include "fd.h"
#include "fdtab.h"
#include "fdtx.h"
#include "comfd.h"

off_t
com_fd_exec_lseek(struct com_fd *data, int fildes, off_t offset, int whence, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Seek */

    int cookie = -1;

    off_t pos = ofdtx_lseek_exec(ofdtx,
                                 fildes, offset, whence,
                                 &cookie, isnoundo);

    if ((long)pos < 0) {
        return pos;
    }

    /* Inject event */
    if ((cookie >= 0) && 
        (com_fd_inject(data, ACTION_LSEEK, fildes, cookie) < 0)) {
        return -1;
    }

    return pos;
}

int
com_fd_apply_lseek(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_lseek_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_lseek(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_lseek_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

