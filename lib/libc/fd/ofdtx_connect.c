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
#include "counter.h"
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

static int
ofdtx_connect_exec_noundo(struct ofdtx *ofdtx, int sockfd,
                                               const struct sockaddr *serv_addr,
                                               socklen_t addrlen,
                                               int *cookie)
{
    return TEMP_FAILURE_RETRY(connect(sockfd, serv_addr, addrlen));
}

static int
ofdtx_connect_exec_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd,
                                                       const struct sockaddr *serv_addr,
                                                       socklen_t addrlen,
                                                       int *cookie)
{
    int type;
    socklen_t typelen = sizeof(type);

    assert(ofdtx);

    /* Exclude non-stream sockets */

    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &typelen) < 0) {
        return ERR_SYSTEM;
    }
    if (type != SOCK_STREAM) {
        return ERR_NOUNDO;
    }

    /* Finish first connect, before second connect can start */
    if (ofdtx->conn) {
        return ERR_NOUNDO;
    }

    /* Create connection */

    struct connection *conn = connection_create(sockfd);

    if (!conn) {
        return ERR_SYSTEM;
    }

    int err = TEMP_FAILURE_RETRY(connect(sockfd, serv_addr, addrlen));

    if (err) {
        connection_destroy(conn);
        return err;
    }

    ofdtx->conn = conn;

    /* Signal apply/undo */
    *cookie = 0;

    return 0;
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
        {ofdtx_connect_exec_noundo, NULL, NULL, ofdtx_connect_exec_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(connect_exec)/sizeof(connect_exec[0]));
    assert(connect_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !connect_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return connect_exec[ofdtx->type][ofdtx->ccmode](ofdtx,
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

static int
ofdtx_connect_apply_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
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
        {ofdtx_connect_apply_noundo, NULL, NULL, ofdtx_connect_apply_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(connect_apply)/sizeof(connect_apply[0]));
    assert(connect_apply[ofdtx->type]);

    return connect_apply[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, event, n);
}

/*
 * Undo
 */

int
ofdtx_connect_undo_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd,
                                                       int cookie)
{
    return TEMP_FAILURE_RETRY(shutdown(sockfd, SHUT_RDWR));
}

int
ofdtx_connect_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const connect_undo[][4])(struct ofdtx*, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, ofdtx_connect_undo_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(connect_undo)/sizeof(connect_undo[0]));
    assert(connect_undo[ofdtx->type]);

    return connect_undo[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, cookie);
}

