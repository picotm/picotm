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
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include "tanger-stm-ext-actions.h"
#include "types.h"
#include "range.h"
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
ofdtx_read_exec_noundo(struct ofdtx *ofdtx, int fildes, void *buf,
                                                      size_t nbyte,
                                                      int *cookie,
                                                      enum validation_mode valmode)
{
    return TEMP_FAILURE_RETRY(read(fildes, buf, nbyte));
}

static ssize_t
ofdtx_read_exec_regular_ts(struct ofdtx *ofdtx, int fildes, void *buf,
                                                      size_t nbyte,
                                                      int *cookie,
                                                      enum validation_mode valmode)
{
    int err;

    assert(ofdtx);

    if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
        return err;
    }
    if ((err = ofdtx_ts_get_region_versions(ofdtx, nbyte, ofdtx->offset)) < 0) {
        return err;
    }

    /* read from file descriptor */
    ssize_t len = pread(fildes, buf, nbyte, ofdtx->offset);

    if (len < 0) {
        return len;
    }

    if (!!ofdtx_ts_validate_state(ofdtx)
        || ((valmode == VALIDATE_OP)
            && !!ofdtx_ts_validate_region(ofdtx, nbyte, ofdtx->offset))) {
        return ERR_CONFLICT;
    }

    /* read from write set */

    ssize_t len2 = iooptab_read(ofdtx->wrtab,
                                ofdtx->wrtablen, buf, nbyte,
                                ofdtx->offset,
                                ofdtx->wrbuf);

    if (len2 < 0) {
        return len2;
    }

    ssize_t res = llmax(len, len2);

    /* register read data for later validation */
    if ((err = ofdtx_append_to_readset(ofdtx, res, ofdtx->offset, NULL)) < 0) {
        return err;
    }

    /* update file pointer */
    ofdtx->offset += res;
    ofdtx->flags  |= OFDTX_FL_LOCALBUF|OFDTX_FL_LOCALSTATE|OFDTX_FL_TL_INCVER;

    return res;
}

static ssize_t
ofdtx_read_exec_regular_2pl(struct ofdtx *ofdtx, int fildes,
                                                 void *buf,
                                                 size_t nbyte,
                                                 int *cookie,
                                                 enum validation_mode valmode)
{
    int err;

    assert(ofdtx);

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* write-lock open file description, because we change the file position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return err;
    }

    /* read-lock region */

    if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, ofdtx->offset, 0)) < 0) {
        return err;
    }

    /* read from file */

    ssize_t len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, ofdtx->offset));

    if (len < 0) {
        return len;
    }

    /* read from local write set */

    ssize_t len2 = iooptab_read(ofdtx->wrtab,
                                ofdtx->wrtablen, buf, nbyte,
                                ofdtx->offset,
                                ofdtx->wrbuf);

    if (len2 < 0) {
        return len2;
    }

    ssize_t res = llmax(len, len2);

    /* update file pointer */
    ofdtx->offset += res;

    return res;
}

static ssize_t
ofdtx_read_exec_socket_2pl_ext(struct ofdtx *ofdtx, int sockfd,
                                                    void *buf,
                                                    size_t nbyte,
                                                    int *cookie,
                                                      enum validation_mode valmode)
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

    /* If this is the first call to this socket within the transaction, no
       connection structure exists yet, therefore create one. */

    if (!ofdtx->conn) {
        ofdtx->conn = connection_create(sockfd);

        if (!ofdtx->conn) {
            return ERR_SYSTEM;
        }
    }

    /* Write-lock socket */
    {
        struct ofd *ofd = ofdtab+ofdtx->ofd;

        int err;

        if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
            return err;
        }
    }

    return connection_recv(ofdtx->conn, buf, nbyte);
}

ssize_t
ofdtx_read_exec(struct ofdtx *ofdtx, int fildes, void *buf,
                                           size_t nbyte,
                                           int *cookie, int noundo,
                                           enum validation_mode valmode)
{
    static ssize_t (* const read_exec[][4])(struct ofdtx*,
                                            int, 
                                            void*,
                                            size_t,
                                            int*,
                                            enum validation_mode) = {
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_read_exec_noundo, ofdtx_read_exec_regular_ts, ofdtx_read_exec_regular_2pl, NULL},
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        ofdtx_read_exec_socket_2pl_ext}};

    assert(ofdtx->type < sizeof(read_exec)/sizeof(read_exec[0]));
    assert(read_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !read_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return read_exec[ofdtx->type][ofdtx->ccmode](ofdtx, fildes, buf, nbyte, cookie, valmode);
}

/*
 * Apply
 */

static ssize_t
ofdtx_read_apply_noundo(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

static ssize_t
ofdtx_read_apply_regular(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    int err = 0;

    while (n && !err) {
        ofdtab[ofdtx->ofd].data.regular.offset += ofdtx->rdtab[event->cookie].nbyte;

        lseek(fildes, ofdtab[ofdtx->ofd].data.regular.offset, SEEK_SET);

        --n;
        ++event;
    }

    return err;
}

static int
ofdtx_read_apply_socket(struct ofdtx *ofdtx, int sockfd, const struct com_fd_event *event, size_t n)
{
    return 0;
}

ssize_t
ofdtx_read_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const read_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_read_apply_noundo, NULL,                     NULL,                     NULL},
        {ofdtx_read_apply_noundo, ofdtx_read_apply_regular, ofdtx_read_apply_regular, NULL},
        {ofdtx_read_apply_noundo, NULL,                     NULL,                     NULL},
        {ofdtx_read_apply_noundo, NULL,                     NULL,                     ofdtx_read_apply_socket}};

    assert(ofdtx->type < sizeof(read_apply)/sizeof(read_apply[0]));
    assert(read_apply[ofdtx->type]);

    return read_apply[ofdtx->type][ofdtx->ccmode](ofdtx, fildes, event, n);
}

/*
 * Undo
 */

static off_t
ofdtx_read_any_undo(void)
{
    return 0;
}

off_t
ofdtx_read_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static off_t (* const read_undo[][4])(void) = {
        {NULL, NULL,                NULL,                NULL},
        {NULL, ofdtx_read_any_undo, ofdtx_read_any_undo, NULL},
        {NULL, NULL,                NULL,                NULL},
        {NULL, NULL,                NULL,                ofdtx_read_any_undo}};

    assert(ofdtx->type < sizeof(read_undo)/sizeof(read_undo[0]));
    assert(read_undo[ofdtx->type]);

    return read_undo[ofdtx->type][ofdtx->ccmode]();
}

