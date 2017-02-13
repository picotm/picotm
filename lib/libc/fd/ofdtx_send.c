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
#include "table.h"
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
#include "fdtx.h"
#include "comfd.h"

/*
 * Exec
 */

static ssize_t
ofdtx_send_exec_noundo(struct ofdtx *ofdtx, int sockfd,
                                      const void *buffer,
                                            size_t length,
                                            int flags,
                                            int *cookie)
{
    return TEMP_FAILURE_RETRY(send(sockfd, buffer, length, flags));
}
static ssize_t
ofdtx_send_exec_socket_ts(struct ofdtx *ofdtx, int sockfd,
                                         const void *buf,
                                               size_t nbyte,
                                               int flags,
                                               int *cookie)
{
    /* Become irrevocable if any flags are selected */
    if (flags) {
        return ERR_NOUNDO;
    }

    /* Register write data */

    if (cookie) {

        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_send_exec_socket_2pl(struct ofdtx *ofdtx, int sockfd,
                                          const void *buf,
                                                size_t nbyte,
                                                int flags,
                                                int *cookie)
{
    int err;

    /* Become irrevocable if any flags are selected */
    if (flags) {
        return ERR_NOUNDO;
    }

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* Write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Register write data */

    if (cookie) {

        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_send_exec_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd,
                                              const void *buffer,
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

    return connection_send(ofdtx->conn, buffer, length);
}

ssize_t
ofdtx_send_exec(struct ofdtx *ofdtx, int sockfd,
                               const void *buffer,
                                     size_t length,
                                     int flags,
                                     int *cookie, int noundo)
{
    static ssize_t (* const send_exec[][4])(struct ofdtx*,
                                            int, 
                                      const void*,
                                            size_t,
                                            int,
                                            int*) = {
        {ofdtx_send_exec_noundo, NULL,                      NULL,                       NULL},
        {ofdtx_send_exec_noundo, NULL,                      NULL,                       NULL},
        {ofdtx_send_exec_noundo, NULL,                      NULL,                       NULL},
        {ofdtx_send_exec_noundo, ofdtx_send_exec_socket_ts, ofdtx_send_exec_socket_2pl, ofdtx_send_exec_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(send_exec)/sizeof(send_exec[0]));
    assert(send_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !send_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return send_exec[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, buffer, length, flags, cookie);
}

/*
 * Apply
 */

static int
ofdtx_send_apply_noundo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    return 0;
}

static int
ofdtx_send_apply_socket_ts(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    assert(ofdtx);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(send(sockfd,
                                ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                ofdtx->wrtab[cookie].nbyte, 0));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

static int
ofdtx_send_apply_socket_2pl(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    assert(ofdtx);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(send(sockfd,
                                ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                ofdtx->wrtab[cookie].nbyte, 0));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

static int
ofdtx_send_apply_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    return 0;
}

int
ofdtx_send_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const send_apply[][4])(struct ofdtx*, int, int) = {
        {ofdtx_send_apply_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_send_apply_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_send_apply_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_send_apply_noundo, ofdtx_send_apply_socket_ts, ofdtx_send_apply_socket_2pl, ofdtx_send_apply_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(send_apply)/sizeof(send_apply[0]));
    assert(send_apply[ofdtx->type][ofdtx->ccmode]);

    int err = 0;

    while (n && !err) {
        err = send_apply[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, event->cookie);
        --n;
        ++event;
    }

    return err;
}

/*
 * Undo
 */

static int
ofdtx_send_undo_socket_ts(void)
{
    return 0;
}

static int
ofdtx_send_undo_socket_2pl(void)
{
    return 0;
}

static int
ofdtx_send_undo_socket_2pl_ext(void)
{
    return 0;
}

int
ofdtx_send_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const send_undo[][4])(void) = {
        {NULL, NULL,                      NULL,                       NULL},
        {NULL, NULL,                      NULL,                       NULL},
        {NULL, NULL,                      NULL,                       NULL},
        {NULL, ofdtx_send_undo_socket_ts, ofdtx_send_undo_socket_2pl, ofdtx_send_undo_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(send_undo)/sizeof(send_undo[0]));
    assert(send_undo[ofdtx->type][ofdtx->ccmode]);

    return send_undo[ofdtx->type][ofdtx->ccmode]();
}

