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
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtab.h"
#include "ioop.h"
#include "iooptab.h"
#include "ofdtx.h"
#include "event.h"

/*
 * Exec
 */

static ssize_t
ofdtx_write_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                       const void *buf,
                                             size_t nbyte,
                                             int *cookie)
{
    return TEMP_FAILURE_RETRY(write(fildes, buf, nbyte));
}

static ssize_t
ofdtx_write_exec_regular_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                 size_t nbyte, int *cookie)
{
    assert(ofdtx);

    /* register write data */

    if (__builtin_expect(!!cookie, 1)) {

        /* add data to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx,
                                                nbyte,
                                                ofdtx->offset, buf)) < 0) {
            return *cookie;
        }
    }

    /* update file pointer */
    ofdtx->offset += nbyte;
    ofdtx->flags  |= OFDTX_FL_TL_INCVER;

    return nbyte;
}

static ssize_t
ofdtx_write_exec_regular_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                   size_t nbyte,
                                                   int *cookie)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        int err;

        struct ofd *ofd = ofdtab+ofdtx->ofd;

        /* write-lock open file description, because we change the file position */

        if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
            return err;
        }

        /* write-lock region */

        if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, ofdtx->offset, 1)) < 0) {
            return err;
        }

        /* add buf to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx,
                                                nbyte,
                                                ofdtx->offset, buf)) < 0) {
            return *cookie;
        }
    }

    /* update file pointer */
    ofdtx->offset += nbyte;

    return nbyte;
}

static ssize_t
ofdtx_write_exec_fifo_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                              size_t nbyte, int *cookie)
{
    /* Register write data */

    if (cookie) {

        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_write_exec_fifo_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                               size_t nbyte,
                                               int *cookie)
{
    int err;

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
ofdtx_write_exec_socket_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                size_t nbyte, int *cookie)
{
    /* Register write data */

    if (cookie) {
        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, 0, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_write_exec_socket_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                 size_t nbyte,
                                                 int *cookie)
{
    int err;

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

ssize_t
ofdtx_write_exec(struct ofdtx *ofdtx, int fildes, const void *buf,
                                      size_t nbyte, int *cookie, int noundo)
{
    static ssize_t (* const write_exec[][4])(struct ofdtx*,
                                             int,
                                             const void*,
                                             size_t,
                                             int*) = {
        {ofdtx_write_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_write_exec_noundo, ofdtx_write_exec_regular_ts, ofdtx_write_exec_regular_2pl, NULL},
        {ofdtx_write_exec_noundo, ofdtx_write_exec_fifo_ts,    ofdtx_write_exec_fifo_2pl,    NULL},
        {ofdtx_write_exec_noundo, ofdtx_write_exec_socket_ts,  ofdtx_write_exec_socket_2pl,  NULL}};

    assert(ofdtx->type < sizeof(write_exec)/sizeof(write_exec[0]));
    assert(write_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == PICOTM_LIBC_CC_MODE_NOUNDO)
            || !write_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return write_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, buf, nbyte, cookie);
}

/*
 * Apply
 */

static int
ofdtx_write_apply_noundo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    return 0;
}

static int
ofdtx_write_apply_regular_ts(struct ofdtx *ofdtx, int fildes, int cookie)
{
    assert(ofdtx);
    assert(fildes >= 0);

    off_t offset;

    /* If no dependencies, use current global offset */
    if (ofdtx->flags&OFDTX_FL_LOCALSTATE) {
        offset = ofdtx->wrtab[cookie].off;
    } else {
        offset = ofd_get_offset_nolock(ofdtab+ofdtx->ofd);
    }

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len = TEMP_FAILURE_RETRY(pwrite(fildes, ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                                          ofdtx->wrtab[cookie].nbyte, offset));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    /* Update file position */
    ofdtab[ofdtx->ofd].data.regular.offset = offset+len;

    lseek(fildes, ofdtab[ofdtx->ofd].data.regular.offset, SEEK_SET);

    return 0;
}

static int
ofdtx_write_apply_regular_2pl(struct ofdtx *ofdtx, int fildes, int cookie)
{
    assert(ofdtx);
    assert(fildes >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len = TEMP_FAILURE_RETRY(pwrite(fildes, ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                                          ofdtx->wrtab[cookie].nbyte,
                                                          ofdtx->wrtab[cookie].off));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    /* Update file position */
    ofdtab[ofdtx->ofd].data.regular.offset = ofdtx->wrtab[cookie].off+len;

    lseek(fildes, ofdtab[ofdtx->ofd].data.regular.offset, SEEK_SET);

    return 0;
}

static int
ofdtx_write_apply_fifo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    assert(ofdtx);
    assert(fildes >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(write(fildes,
                                 ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                 ofdtx->wrtab[cookie].nbyte));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

static int
ofdtx_write_apply_socket(struct ofdtx *ofdtx, int sockfd, int cookie)
{
    assert(ofdtx);
    assert(sockfd >= 0);

    /* FIXME: Use select() to prevent blocking? */

    const ssize_t len =
        TEMP_FAILURE_RETRY(write(sockfd,
                                 ofdtx->wrbuf+ofdtx->wrtab[cookie].bufoff,
                                 ofdtx->wrtab[cookie].nbyte));

    if (len < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

ssize_t
ofdtx_write_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static int (* const write_apply[][4])(struct ofdtx*, int, int) = {
        {ofdtx_write_apply_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_write_apply_noundo, ofdtx_write_apply_regular_ts, ofdtx_write_apply_regular_2pl, NULL},
        {ofdtx_write_apply_noundo, ofdtx_write_apply_fifo,       ofdtx_write_apply_fifo,        NULL},
        {ofdtx_write_apply_noundo, ofdtx_write_apply_socket,     ofdtx_write_apply_socket,      NULL}};

    assert(ofdtx->type < sizeof(write_apply)/sizeof(write_apply[0]));
    assert(write_apply[ofdtx->type][ofdtx->cc_mode]);

    int err = 0;

    while (n && !err) {
        err = write_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, event->cookie);
        --n;
        ++event;
    }

    return err;
}

/*
 * Undo
 */

static int
ofdtx_write_any_undo(void)
{
    return 0;
}

int
ofdtx_write_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const write_undo[][4])(void) = {
        {NULL, NULL,                 NULL,                 NULL},
        {NULL, ofdtx_write_any_undo, ofdtx_write_any_undo, NULL},
        {NULL, ofdtx_write_any_undo, ofdtx_write_any_undo, NULL},
        {NULL, ofdtx_write_any_undo, ofdtx_write_any_undo, ofdtx_write_any_undo}};

    assert(ofdtx->type < sizeof(write_undo)/sizeof(write_undo[0]));
    assert(write_undo[ofdtx->type][ofdtx->cc_mode]);

    return write_undo[ofdtx->type][ofdtx->cc_mode]();
}

