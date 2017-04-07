/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include "tanger-stm-ext-actions.h"
#include "types.h"
#include "counter.h"
#include "rwlock.h"
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
#include "ioop.h"
#include "iooptab.h"
#include "ofdtx.h"

/*
 * Exec
 */

static int
ofdtx_shutdown_exec_noundo(struct ofdtx *ofdtx, int sockfd, int how,
                                                            int *cookie)
{
    return shutdown(sockfd, how);
}

int
ofdtx_shutdown_exec(struct ofdtx *ofdtx, int sockfd,
                                         int how,
                                         int *cookie,
                                         int noundo)
{
    static int (* const shutdown_exec[][4])(struct ofdtx*,
                                            int,
                                            int,
                                            int*) = {
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_exec_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(shutdown_exec)/sizeof(shutdown_exec[0]));
    assert(shutdown_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !shutdown_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return shutdown_exec[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, how, cookie);
}

/*
 * Apply
 */

static int
ofdtx_shutdown_apply_noundo(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

int
ofdtx_shutdown_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const shutdown_apply[][4])(struct ofdtx*,
                                             int,
                                             const struct com_fd_event*, size_t) = {
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL},
        {ofdtx_shutdown_apply_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(shutdown_apply)/sizeof(shutdown_apply[0]));
    assert(shutdown_apply[ofdtx->type]);

    return shutdown_apply[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, event, n);
}

/*
 * Undo
 */

int
ofdtx_shutdown_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const shutdown_undo[][4])(struct ofdtx*, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(shutdown_undo)/sizeof(shutdown_undo[0]));
    assert(shutdown_undo[ofdtx->type]);

    return shutdown_undo[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, cookie);
}

