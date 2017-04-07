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
#include "ofdid.h"
#include "ofd.h"
#include "ofdtab.h"
#include "ofdtx.h"

/*
 * Exec
 */

static int
ofdtx_fsync_exec_noundo(int fildes, int *cookie)
{
    return fsync(fildes);
}

static int
ofdtx_fsync_exec_regular_ts(int fildes, int *cookie)
{
    /* Signal apply/undo */
    *cookie = 0;

    return 0;
}

static int
ofdtx_fsync_exec_regular_2pl(int fildes, int *cookie)
{
    /* Signal apply/undo */
    *cookie = 0;

    return 0;
}

int
ofdtx_fsync_exec(struct ofdtx *ofdtx, int fildes, int noundo, int *cookie)
{
    static int (* const fsync_exec[][4])(int, int*) = {
        {ofdtx_fsync_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_fsync_exec_noundo, ofdtx_fsync_exec_regular_ts, ofdtx_fsync_exec_regular_2pl, NULL},
        {ofdtx_fsync_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_fsync_exec_noundo, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(fsync_exec)/sizeof(fsync_exec[0]));

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = SYSTX_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == SYSTX_LIBC_CC_MODE_NOUNDO)
            || !fsync_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return fsync_exec[ofdtx->type][ofdtx->cc_mode](fildes, cookie);
}

/*
 * Apply
 */

static int
ofdtx_fsync_apply_noundo(int fildes)
{
    return 0;
}

static int
ofdtx_fsync_apply_regular_ts(int fildes)
{
    return fsync(fildes);
}

static int
ofdtx_fsync_apply_regular_2pl(int fildes)
{
    return fsync(fildes);
}

int
ofdtx_fsync_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static int (* const fsync_apply[][4])(int) = {
        {ofdtx_fsync_apply_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_fsync_apply_noundo, ofdtx_fsync_apply_regular_ts, ofdtx_fsync_apply_regular_2pl, NULL},
        {ofdtx_fsync_apply_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_fsync_apply_noundo, NULL,                         NULL,                          NULL}};

    assert(ofdtx->type < sizeof(fsync_apply)/sizeof(fsync_apply[0]));
    assert(fsync_apply[ofdtx->type][ofdtx->cc_mode]);

    return fsync_apply[ofdtx->type][ofdtx->cc_mode](fildes);
}

/*
 * Undo
 */

static int
ofdtx_fsync_undo_regular_ts(int fildes, int cookie)
{
    return 0;
}

static int
ofdtx_fsync_undo_regular_2pl(int fildes, int cookie)
{
    return 0;
}

int
ofdtx_fsync_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const fsync_undo[][4])(int, int) = {
        {NULL, NULL,                        NULL,                         NULL},
        {NULL, ofdtx_fsync_undo_regular_ts, ofdtx_fsync_undo_regular_2pl, NULL},
        {NULL, NULL,                        NULL,                         NULL},
        {NULL, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(fsync_undo)/sizeof(fsync_undo[0]));
    assert(fsync_undo[ofdtx->type][ofdtx->cc_mode]);

    return fsync_undo[ofdtx->type][ofdtx->cc_mode](fildes, cookie);
}

