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
#include "connection.h"
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

static ssize_t
ofdtx_recv_exec_noundo(struct ofdtx *ofdtx, int sockfd,
                                            void *buffer,
                                            size_t length,
                                            int flags,
                                            int *cookie)
{
    return TEMP_FAILURE_RETRY(recv(sockfd, buffer, length, flags));
}

static ssize_t
ofdtx_recv_exec_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd,
                                                    void *buffer,
                                                    size_t length,
                                                    int flags,
                                                    int *cookie)
{
    int type;
    socklen_t typelen = sizeof(type);

    assert(ofdtx);

    if (flags) {
        return ERR_NOUNDO;
    }

    /* Exclude non-stream sockets */

    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &typelen) < 0) {
        return ERR_SYSTEM;
    }
    if (type != SOCK_STREAM) {
        return ERR_NOUNDO;
    }

    /* If this is the first call to this socket within the transaction, no
       connection structure exists yet, therefore create one. */

    if (!ofdtx->conn) {
        ofdtx->conn = connection_create(sockfd);

        if (!ofdtx->conn) {
            return ERR_SYSTEM;
        }
    }

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* Write-lock socket */

    int err;

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    return connection_recv(ofdtx->conn, buffer, length);
}

ssize_t
ofdtx_recv_exec(struct ofdtx *ofdtx, int sockfd,
                                     void *buffer,
                                     size_t length,
                                     int flags,
                                     int *cookie, int noundo)
{
    static ssize_t (* const recv_exec[][4])(struct ofdtx*,
                                            int, 
                                            void*,
                                            size_t,
                                            int,
                                            int*) = {
        {ofdtx_recv_exec_noundo, NULL, NULL, NULL},
        {ofdtx_recv_exec_noundo, NULL, NULL, NULL},
        {ofdtx_recv_exec_noundo, NULL, NULL, NULL},
        {ofdtx_recv_exec_noundo, NULL, NULL, ofdtx_recv_exec_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(recv_exec)/sizeof(recv_exec[0]));
    assert(recv_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !recv_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return recv_exec[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, buffer, length, flags, cookie);
}

/*
 * Apply
 */

static int
ofdtx_recv_apply_noundo(void)
{
    return 0;
}

static int
ofdtx_recv_apply_socket_2pl_ext(void)
{
    return 0;
}

int
ofdtx_recv_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const recv_apply[][4])(void) = {
        {ofdtx_recv_apply_noundo, NULL, NULL, NULL},
        {ofdtx_recv_apply_noundo, NULL, NULL, NULL},
        {ofdtx_recv_apply_noundo, NULL, NULL, NULL},
        {ofdtx_recv_apply_noundo, NULL, NULL, ofdtx_recv_apply_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(recv_apply)/sizeof(recv_apply[0]));
    assert(recv_apply[ofdtx->type]);

    return recv_apply[ofdtx->type][ofdtx->ccmode]();
}

/*
 * Undo
 */

static int
ofdtx_recv_undo_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    return 0;
}

int
ofdtx_recv_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const recv_undo[][4])(struct ofdtx *, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, ofdtx_recv_undo_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(recv_undo)/sizeof(recv_undo[0]));
    assert(recv_undo[ofdtx->type]);

    return recv_undo[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, cookie);
}

