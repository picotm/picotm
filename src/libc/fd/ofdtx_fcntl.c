/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include "tanger-stm-ext-actions.h"
#include "types.h"
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
#include "fcntloptab.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtab.h"
#include "ofdtx.h"

/*
 * Exec
 */

int
ofdtx_fcntl_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                             int cmd, union com_fd_fcntl_arg *arg,
                                             int *cookie)
{
    int res = 0;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));

            if (arg->arg0 < 0) {
                res = -1;
            }
            break;
        case F_GETLK:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg0));
            break;
        case F_SETLK:
        case F_SETLKW:
            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        default:
            errno = EINVAL;
            res = ERR_SYSTEM;
            break;
    }

    return res;
}

int
ofdtx_fcntl_exec_ts(struct ofdtx *ofdtx, int fildes,
                                         int cmd, union com_fd_fcntl_arg *arg,
                                         int *cookie)
{
    int res = 0;
    int err, valid;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            /* fetch version number */
            if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
                return err;
            }

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            if (arg->arg0 < 0) {
                res = ERR_SYSTEM;
                break;
            }

            /* validate, as fcntl might have overlapped with commit */
            valid = !ofdtx_ts_validate_state(ofdtx);

            if (!valid) {
                return ERR_CONFLICT;
            }
            break;
        case F_GETLK:

            /* fetch version number */
            if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
                return err;
            }

            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));

            if (!(res < 0)) {
                break;
            }

            /* validate, as fcntl might have overlapped with commit */
            valid = !ofdtx_ts_validate_state(ofdtx);

            if (!valid) {
                return ERR_CONFLICT;
            }
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
        case F_SETLK:
        case F_SETLKW:
            res = ERR_NOUNDO;
            break;
        default:
            errno = EINVAL;
            res = ERR_SYSTEM;
            break;
    }

    if (res < 0) {
        return res;
    }

	ofdtx->flags |= OFDTX_FL_LOCALSTATE;

    return res;
}

int
ofdtx_fcntl_exec_2pl(struct ofdtx *ofdtx, int fildes,
                                          int cmd, union com_fd_fcntl_arg *arg,
                                          int *cookie)
{
    int res = 0;

    assert(arg);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN:

            /* Read-lock open file description */
            if ((res = ofd_rdlock_state(ofdtab+ofdtx->ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
                return res;
            }

            arg->arg0 = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            if (arg->arg0 < 0) {
                res = -1;
            }
            break;
        case F_GETLK:

            /* Read-lock open file description */
            if ((res = ofd_rdlock_state(ofdtab+ofdtx->ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
                return res;
            }

            res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            break;
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN:
        case F_SETLK:
        case F_SETLKW:
            res = ERR_NOUNDO;
            break;
        default:
            errno = EINVAL;
            res = ERR_SYSTEM;
            break;
    }

    if (res < 0) {
        return res;
    }

    return res;
}

int
ofdtx_fcntl_exec(struct ofdtx *ofdtx, int fildes,
                                      int cmd, union com_fd_fcntl_arg *arg,
                                      int *cookie, int noundo)
{
    static int (* const fcntl_exec[][4])(struct ofdtx*,
                                         int,
                                         int,
                                         union com_fd_fcntl_arg*, int*) = {
        {ofdtx_fcntl_exec_noundo, NULL,                NULL,                 NULL},
        {ofdtx_fcntl_exec_noundo, ofdtx_fcntl_exec_ts, ofdtx_fcntl_exec_2pl, NULL},
        {ofdtx_fcntl_exec_noundo, ofdtx_fcntl_exec_ts, ofdtx_fcntl_exec_2pl, NULL},
        {ofdtx_fcntl_exec_noundo, NULL,                NULL,                 NULL}};

    assert(ofdtx->type < sizeof(fcntl_exec)/sizeof(fcntl_exec[0]));
    assert(fcntl_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = SYSTX_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == SYSTX_LIBC_CC_MODE_NOUNDO)
            || !fcntl_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return fcntl_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, cmd, arg, cookie);
}

/*
 * Apply
 */

int
ofdtx_fcntl_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

/*
 * Undo
 */

int
ofdtx_fcntl_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    return 0;
}

