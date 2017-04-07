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
ofdtx_listen_exec_noundo(struct ofdtx *ofdtx, int sockfd, int backlog, int *cookie)
{
    return listen(sockfd, backlog);
}

static int
ofdtx_listen_exec_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, int backlog, int *cookie)
{
    int type;
    socklen_t typelen = sizeof(type);

    assert(ofdtx);

    /* Write-lock socket */

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    int err;

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* Exclude non-stream sockets */

    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &typelen) < 0) {
        return ERR_SYSTEM;
    }
    if (type != SOCK_STREAM) {
        return ERR_NOUNDO;
    }

    /* If the socket is in blocking mode, the transaction might wait forever
       and therefore block the whole system. Therefore only listen for a given
       amount of time and abort if no connection request was received. */

    int fl = fcntl(sockfd, F_GETFL);

    if (fl < 0) {
        return ERR_SYSTEM;
    } else if (fl&O_NONBLOCK) {

        fd_set rdset;
        FD_ZERO(&rdset);
        FD_SET(sockfd, &rdset);

        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        int numfds = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &rdset, NULL, NULL, &tv));

        if (numfds < 0) {
            return ERR_SYSTEM;
        }

        int isset = FD_ISSET(sockfd, &rdset);

        FD_ZERO(&rdset);

        if (!isset) {
            return ERR_CONFLICT;
        }
    }

    return listen(sockfd, backlog);
}

int
ofdtx_listen_exec(struct ofdtx *ofdtx, int sockfd,
                                       int backlog,
                                       int *cookie,
                                       int noundo)
{
    static int (* const listen_exec[][4])(struct ofdtx*,
                                          int,
                                          int,
                                          int*) = {
        {ofdtx_listen_exec_noundo, NULL, NULL, NULL},
        {ofdtx_listen_exec_noundo, NULL, NULL, NULL},
        {ofdtx_listen_exec_noundo, NULL, NULL, NULL},
        {ofdtx_listen_exec_noundo, NULL, NULL, ofdtx_listen_exec_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(listen_exec)/sizeof(listen_exec[0]));
    assert(listen_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !listen_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return listen_exec[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, backlog, cookie);
}

/*
 * Apply
 */

static int
ofdtx_listen_apply_noundo()
{
    return 0;
}

static int
ofdtx_listen_apply_socket_2pl_ext()
{
    return 0;
}

int
ofdtx_listen_apply(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    static int (* const listen_apply[][4])(void) = {
        {ofdtx_listen_apply_noundo, NULL, NULL, NULL},
        {ofdtx_listen_apply_noundo, NULL, NULL, NULL},
        {ofdtx_listen_apply_noundo, NULL, NULL, NULL},
        {ofdtx_listen_apply_noundo, NULL, NULL, ofdtx_listen_apply_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(listen_apply)/sizeof(listen_apply[0]));
    assert(listen_apply[ofdtx->type]);

    return listen_apply[ofdtx->type][ofdtx->ccmode]();
}

/*
 * Undo
 */

int
ofdtx_listen_undo_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    return 0;
}

int
ofdtx_listen_undo(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    static int (* const listen_undo[][4])(struct ofdtx*, int, int) = {
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, NULL},
        {NULL, NULL, NULL, ofdtx_listen_undo_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(listen_undo)/sizeof(listen_undo[0]));
    assert(listen_undo[ofdtx->type]);

    return listen_undo[ofdtx->type][ofdtx->ccmode](ofdtx, sockfd, cookie);
}

