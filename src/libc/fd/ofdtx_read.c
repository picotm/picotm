/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
                                                      enum systx_libc_validation_mode val_mode)
{
    return TEMP_FAILURE_RETRY(read(fildes, buf, nbyte));
}

static ssize_t
ofdtx_read_exec_regular_ts(struct ofdtx *ofdtx, int fildes, void *buf,
                                                      size_t nbyte,
                                                      int *cookie,
                                                      enum systx_libc_validation_mode val_mode)
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
        || ((val_mode == SYSTX_LIBC_VALIDATE_OP)
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
                                                 enum systx_libc_validation_mode val_mode)
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

ssize_t
ofdtx_read_exec(struct ofdtx *ofdtx, int fildes, void *buf,
                                           size_t nbyte,
                                           int *cookie, int noundo,
                                           enum systx_libc_validation_mode val_mode)
{
    static ssize_t (* const read_exec[][4])(struct ofdtx*,
                                            int,
                                            void*,
                                            size_t,
                                            int*,
                                            enum systx_libc_validation_mode) = {
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_read_exec_noundo, ofdtx_read_exec_regular_ts, ofdtx_read_exec_regular_2pl, NULL},
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL},
        {ofdtx_read_exec_noundo, NULL,                       NULL,                        NULL}};

    assert(ofdtx->type < sizeof(read_exec)/sizeof(read_exec[0]));
    assert(read_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->cc_mode = SYSTX_LIBC_CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->cc_mode == SYSTX_LIBC_CC_MODE_NOUNDO)
            || !read_exec[ofdtx->type][ofdtx->cc_mode]) {
            return ERR_NOUNDO;
        }
    }

    return read_exec[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, buf, nbyte, cookie, val_mode);
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

static ssize_t
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

    return read_apply[ofdtx->type][ofdtx->cc_mode](ofdtx, fildes, event, n);
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

    return read_undo[ofdtx->type][ofdtx->cc_mode]();
}

