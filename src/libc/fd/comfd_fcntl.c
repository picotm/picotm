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
com_fd_exec_fcntl(struct com_fd *data, int fildes, int cmd, union com_fd_fcntl_arg *arg, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Fcntl */

    int cookie = -1;

    int res = fdtx_fcntl_exec(fdtx, cmd, arg, &cookie, isnoundo);

    if (res < 0) {
        if (res == ERR_DOMAIN) {
            /* Do nothing */
        } else {
            return res;
        }
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

    /* Fcntl */

    res = ofdtx_fcntl_exec(ofdtx, fildes, cmd, arg, &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_FCNTL, fildes, cookie) < 0)) {
        return -1;
    }

    return res;
}

int
com_fd_apply_fcntl(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        int res = fdtx_fcntl_apply(fdtx, event->cookie);

        if (res == ERR_DOMAIN)  {

            struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
            assert(ofdtx);

            res = ofdtx_fcntl_apply(ofdtx, event->fildes, event, 1);
        }

        err = res < 0 ? -1 : 0;
        --n;
        ++event;
    }

    return err;
}

int
com_fd_undo_fcntl(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    int res = fdtx_fcntl_undo(fdtx, cookie);

    if (res == ERR_DOMAIN)  {

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        res = ofdtx_fcntl_undo(ofdtx, fildes, cookie);
    }

    return res < 0 ? -1 : 0;
}

