/* Copyright (C) 2009  Thomas Zimmermann
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
ofdtx_bind_exec_noundo(struct ofdtx *ofdtx, int sockfd, const struct sockaddr *addr, socklen_t addrlen, int *cookie)
{
    return bind(sockfd, addr, addrlen);
}

int
ofdtx_bind_exec(struct ofdtx *ofdtx, int sockfd,
                               const struct sockaddr *addr,
                                     socklen_t addrlen,
                                     int *cookie,
                                     int noundo)
{
    static int (* const bind_exec[][4])(struct ofdtx*,
                                        int,
                                  const struct sockaddr*,
                                        socklen_t,
                                        int*) = {
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL},
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL},
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL},
        {ofdtx_bind_exec_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(bind_exec)/sizeof(bind_exec[0]));
    assert(bind_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !bind_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return bind_exec[ofdtx->type][ofdtx->ccmode](ofdtx,
                                                 sockfd,
                                                 addr,
                                                 addrlen, cookie);
}

/*
 * Apply
 */

static int
ofdtx_bind_apply_noundo(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

int
ofdtx_bind_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const bind_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL},
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL},
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL},
        {ofdtx_bind_apply_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(bind_apply)/sizeof(bind_apply[0]));
    assert(bind_apply[ofdtx->type]);

    return bind_apply[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, event, n);
}

/*
 * Undo
 */

int
ofdtx_bind_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const bind_undo[][4])(int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(bind_undo)/sizeof(bind_undo[0]));
    assert(bind_undo[ofdtx->type]);

    return bind_undo[ofdtx->type][ofdtx->ccmode](cookie);
}

