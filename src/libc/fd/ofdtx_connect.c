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
#include "errcode.h"
#include "types.h"
#include "counter.h"
#include "rwlock.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "counter.h"
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
ofdtx_connect_exec_noundo(struct ofdtx *ofdtx, int sockfd,
                                               const struct sockaddr *serv_addr,
                                               socklen_t addrlen,
                                               int *cookie)
{
    return TEMP_FAILURE_RETRY(connect(sockfd, serv_addr, addrlen));
}

int
ofdtx_connect_exec(struct ofdtx *ofdtx, int sockfd,
                                        const struct sockaddr *serv_addr,
                                        socklen_t addrlen,
                                        int *cookie,
                                        int noundo)
{
    static int (* const connect_exec[][4])(struct ofdtx*,
                                           int,
                                           const struct sockaddr*,
                                           socklen_t,
                                           int*) = {
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL},
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL},
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL},
        {ofdtx_connect_exec_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(connect_exec)/sizeof(connect_exec[0]));
    assert(connect_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = SYSTX_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == SYSTX_LIBC_CC_MODE_NOUNDO)
            || !connect_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return connect_exec[ofdtx->type][ofdtx->cc_mode](ofdtx,
                                                     sockfd,
                                                     serv_addr,
                                                     addrlen, cookie);
}

/*
 * Apply
 */

static int
ofdtx_connect_apply_noundo(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

int
ofdtx_connect_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const connect_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL},
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL},
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL},
        {ofdtx_connect_apply_noundo, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(connect_apply)/sizeof(connect_apply[0]));
    assert(connect_apply[ofdtx->type]);

    return connect_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, event, n);
}

/*
 * Undo
 */

int
ofdtx_connect_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const connect_undo[][4])(struct ofdtx*, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL}};

    assert(ofdtx->type < sizeof(connect_undo)/sizeof(connect_undo[0]));
    assert(connect_undo[ofdtx->type]);

    return connect_undo[ofdtx->type][ofdtx->cc_mode](ofdtx, sockfd, cookie);
}

