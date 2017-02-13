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
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include "tanger-stm-ext-actions.h"
#include "types.h"
#include "range.h"
#include "mutex.h"
#include "counter.h"
#include "rwlock.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "seekop.h"
#include "seekoptab.h"
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtab.h"
#include "ofdtx.h"
#include "fdtx.h"
#include "comfd.h"

static off_t
filesize(int fildes)
{
    struct stat buf;

    if (fstat(fildes, &buf) < 0) {
        return ERR_SYSTEM;
    }

    return buf.st_size;
}

/*
 * Exec
 */

static off_t
ofdtx_lseek_exec_noundo(struct ofdtx *ofdtx, int fildes,
                               off_t offset, int whence,
                                  int *cookie)
{
    return TEMP_FAILURE_RETRY(lseek(fildes, offset, whence));
}

static off_t
ofdtx_lseek_exec_regular_ts(struct ofdtx *ofdtx, int fildes,
                                                 off_t offset, int whence, 
                                                 int *cookie)
{
    int err;

	/* fastpath: read current position */
	if (!offset && (whence == SEEK_CUR)) {
		ofdtx->flags |= OFDTX_FL_LOCALSTATE;
		return ofdtx->offset;
	}

    ofdtx->size = llmax(ofdtx->offset, ofdtx->size);

    /* fetch version number */
    if ((err = ofdtx_ts_get_state_version(ofdtx)) < 0) {
        return err;
    }

    /* Compute absolute position */

    off_t pos;

    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = ofdtx->offset + offset;
            break;
        case SEEK_END:
            {
                const off_t fs = filesize(fildes);

                if (fs == (off_t)-1) {
                    pos = (off_t)ERR_SYSTEM;
                    break;
                }

                pos = llmax(ofdtx->size, fs)+offset;

                /* validate, as file size might have changed */
                int valid = !ofdtx_ts_validate_state(ofdtx);

                if (!valid) {
                    return ERR_CONFLICT;
                }
            }
            break;
        default:
            pos = -1;
            break;
    }

    if (pos < 0) {
        errno = EINVAL;
        pos = (off_t)ERR_SYSTEM;
    }

    if ((pos == (off_t)-2) || (pos == (off_t)-1)) {
        return pos;
    }

    if (cookie) {
        *cookie = seekoptab_append(&ofdtx->seektab,
                                   &ofdtx->seektablen,
                                    ofdtx->offset, offset, whence);

        if (*cookie < 0) {
            abort();
        }
    }

    ofdtx->offset = pos; /* update file pointer */
	ofdtx->flags |= OFDTX_FL_LOCALSTATE|OFDTX_FL_TL_INCVER;

    return pos;
}
static off_t
ofdtx_lseek_exec_regular_2pl(struct ofdtx *ofdtx, int fildes,
                                         off_t offset, int whence, 
                                             int *cookie)
{
    int err;

    struct ofd *ofd = ofdtab+ofdtx->ofd;

    /* Read-lock open file description */

    if ((err = ofd_rdlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return (off_t)err;
    }

	/* Fastpath: Read current position */
	if (!offset && (whence == SEEK_CUR)) {
		return ofdtx->offset;
	}

    /* Write-lock open file description to change position */

    if ((err = ofd_wrlock_state(ofd, &ofdtx->modedata.tpl.rwstate)) < 0) {
        return (off_t)err;
    }

    /* Compute absolute position */

    ofdtx->size = llmax(ofdtx->offset, ofdtx->size);

    off_t pos;

    switch (whence) {
        case SEEK_SET:
            pos = offset;
            break;
        case SEEK_CUR:
            pos = ofdtx->offset + offset;
            break;
        case SEEK_END:
            {
                const off_t fs = filesize(fildes);

                if (fs == (off_t)ERR_SYSTEM) {
                    pos = (off_t)ERR_SYSTEM;
                    break;
                }

                pos = llmax(ofdtx->size, fs)+offset;
            }
            break;
        default:
            pos = -1;
            break;
    }

    if (pos < 0) {
        errno = EINVAL;
        pos = (off_t)ERR_SYSTEM;
    }

    if ((pos == (off_t)-2) || (pos == (off_t)-1)) {
        return pos;
    }

    if (cookie) {
        *cookie = seekoptab_append(&ofdtx->seektab,
                                   &ofdtx->seektablen,
                                    ofdtx->offset, offset, whence);

        if (*cookie < 0) {
            abort();
        }
    }

    ofdtx->offset = pos; /* Update file pointer */

    return pos;
}

static off_t
ofdtx_lseek_exec_fifo_ts(struct ofdtx *ofdtx, int fildes, 
                                              off_t offset, int whence,
                                              int *cookie)
{
    errno = EPIPE;

    return ERR_SYSTEM;
}

off_t
ofdtx_lseek_exec(struct ofdtx *ofdtx, int fildes,  off_t offset,
                                            int whence, int *cookie,
                                            int noundo)
{
    static off_t (* const lseek_exec[][4])(struct ofdtx*,
                                           int,
                                           off_t,
                                           int,
                                           int*) = {
        {ofdtx_lseek_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_lseek_exec_noundo, ofdtx_lseek_exec_regular_ts, ofdtx_lseek_exec_regular_2pl, NULL},
        {ofdtx_lseek_exec_noundo, ofdtx_lseek_exec_fifo_ts,    NULL,                         NULL},
        {ofdtx_lseek_exec_noundo, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(lseek_exec)/sizeof(lseek_exec[0]));
    assert(lseek_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !lseek_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return lseek_exec[ofdtx->type][ofdtx->ccmode](ofdtx, fildes, offset, whence, cookie);
}

/*
 * Apply
 */

static int
ofdtx_lseek_apply_noundo(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

static int
ofdtx_lseek_apply_regular(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    int err = 0;

    while (n && !err) {
        const off_t pos = lseek(fildes, ofdtx->seektab[event->cookie].offset,
                                        ofdtx->seektab[event->cookie].whence);

        if (pos == (off_t)-1) {
            err = ERR_SYSTEM;
            break;
        }

        struct ofd *ofd = ofdtab+ofdtx->ofd;

        ofd->data.regular.offset = pos;

        --n;
        ++event;
    }

    return err;
}

int
ofdtx_lseek_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static int (* const lseek_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_lseek_apply_noundo, NULL,                      NULL,                      NULL},
        {ofdtx_lseek_apply_noundo, ofdtx_lseek_apply_regular, ofdtx_lseek_apply_regular, NULL},
        {ofdtx_lseek_apply_noundo, NULL,                      NULL,                      NULL},
        {ofdtx_lseek_apply_noundo, NULL,                      NULL,                      NULL}};

    assert(ofdtx->type < sizeof(lseek_apply)/sizeof(lseek_apply[0]));
    assert(lseek_apply[ofdtx->type][ofdtx->ccmode]);

    return lseek_apply[ofdtx->type][ofdtx->ccmode](ofdtx, fildes, event, n);
}

/*
 * Undo
 */

static int
ofdtx_lseek_undo_regular(void)
{
    return 0;
}

int
ofdtx_lseek_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const lseek_undo[][4])() = {
        {NULL, NULL,                     NULL,                     NULL},
        {NULL, ofdtx_lseek_undo_regular, ofdtx_lseek_undo_regular, NULL},
        {NULL, NULL,                     NULL,                     NULL},
        {NULL, NULL,                     NULL,                     NULL}};

    assert(ofdtx->type < sizeof(lseek_undo)/sizeof(lseek_undo[0]));
    assert(lseek_undo[ofdtx->type][ofdtx->ccmode]);

    return lseek_undo[ofdtx->type][ofdtx->ccmode]();
}

