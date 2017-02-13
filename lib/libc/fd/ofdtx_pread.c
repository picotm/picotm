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
#include "rwlock.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "fcntlop.h"
#include "ioop.h"
#include "iooptab.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtab.h"
#include "ofdtx.h"

/*
 * Exec
 */

/*

Busy-wait instead of syscall

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d; 
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d)); 
     return ((ticks)a) | (((ticks)d) << 32); 
}

static ssize_t
pread_wait(int fildes, void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    memset(buf, 0, nbyte);
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 631 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1020 + (339*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/


static ssize_t
ofdtx_pread_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                             void *buf,
                                             size_t nbyte,
                                             off_t offset,
                                             int *cookie,
                                             enum validation_mode valmode)
{
    return TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset));
}

static ssize_t
ofdtx_pread_exec_regular_ts(struct ofdtx *ofdtx, int fildes, void *buf,
                                                 size_t nbyte, off_t offset,
                                                 int *cookie,
                                                 enum validation_mode valmode)
{
    int err;
    ssize_t len, len2;

    assert(ofdtx);

    if ((err = ofdtx_ts_get_region_versions(ofdtx, nbyte, offset)) < 0) {
        return err;
    }

    if ((len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset))) < 0) {
        return len;
    }

    /* signal conflict, if global version numbers changed */
    if ((valmode == VALIDATE_OP) &&
        !!ofdtx_ts_validate_region(ofdtx, nbyte, offset)) {
        return ERR_CONFLICT;
    }

    if ((len2 = iooptab_read(ofdtx->wrtab,
                             ofdtx->wrtablen, buf, nbyte, offset,
                             ofdtx->wrbuf)) < 0) {
        return len2;
    }

    ssize_t res = llmax(len, len2);

    /* add to read set for later validation */
    if ((err = ofdtx_append_to_readset(ofdtx, res, offset, NULL)) < 0) {
        return err;
    }

    ofdtx->flags |= OFDTX_FL_LOCALBUF;

    return res;
}

static ssize_t
ofdtx_pread_exec_regular_2pl(struct ofdtx *ofdtx, int fildes, void *buf,
                                                  size_t nbyte, off_t offset,
                                                  int *cookie,
                                                  enum validation_mode valmode)
{
    int err;
    ssize_t len, len2;

    /* lock region */
    if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, offset, 0)) < 0) {
        return err;
    }

    /* read from file */
    if ((len = TEMP_FAILURE_RETRY(pread(fildes, buf, nbyte, offset))) < 0) {
        return len;
    }

    /* read from local write set */
    if ((len2 = iooptab_read(ofdtx->wrtab,
                             ofdtx->wrtablen,
                             buf, nbyte, offset, ofdtx->wrbuf)) < 0) {
        return len2;
    }

    return llmax(len, len2);
}

static ssize_t
ofdtx_pread_exec_fifo(struct ofdtx *ofdtx, int fildes, void *buf, size_t nbyte,
                                            off_t offset, int *cookie,
                                            enum validation_mode valmode)
{
    errno = ESPIPE;
    return ERR_SYSTEM;
}

ssize_t
ofdtx_pread_exec(struct ofdtx *ofdtx, int fildes, void *buf,
                                      size_t nbyte, off_t offset,
                                      int *cookie, int noundo,
                                      enum validation_mode valmode)
{
    static ssize_t (* const pread_exec[][4])(struct ofdtx*,
                                             int,
                                             void*,
                                             size_t,
                                             off_t,
                                             int*,
                                             enum validation_mode) = {
        {ofdtx_pread_exec_noundo, NULL,                        NULL,                         NULL},
        {ofdtx_pread_exec_noundo, ofdtx_pread_exec_regular_ts, ofdtx_pread_exec_regular_2pl, NULL},
        {ofdtx_pread_exec_noundo, ofdtx_pread_exec_fifo,       ofdtx_pread_exec_fifo,        NULL},
        {ofdtx_pread_exec_noundo, NULL,                        NULL,                         NULL}};

    assert(ofdtx->type < sizeof(pread_exec)/sizeof(pread_exec[0]));
    assert(pread_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !pread_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return pread_exec[ofdtx->type][ofdtx->ccmode](ofdtx, fildes, buf, nbyte, offset, cookie, valmode);
}

/*
 * Apply
 */

static ssize_t
ofdtx_pread_apply_any(void)
{
    return 0;
}

ssize_t
ofdtx_pread_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const pread_apply[][4])(void) = {
        {ofdtx_pread_apply_any, NULL,                  NULL,                  NULL},
        {ofdtx_pread_apply_any, ofdtx_pread_apply_any, ofdtx_pread_apply_any, NULL},
        {ofdtx_pread_apply_any, NULL,                  NULL,                  NULL},
        {ofdtx_pread_apply_any, NULL,                  NULL,                  NULL}};

    assert(ofdtx->type < sizeof(pread_apply)/sizeof(pread_apply[0]));
    assert(pread_apply[ofdtx->type]);

    int err;

    while (n && !err) {
        err = pread_apply[ofdtx->type][ofdtx->ccmode]();
        --n;
    }

    return err;
}

/*
 * Undo
 */

static int
ofdtx_pread_undo_any(void)
{
    return 0;
}

int
ofdtx_pread_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static int (* const pread_undo[][4])(void) = {
        {NULL, NULL,                 NULL,                 NULL},
        {NULL, ofdtx_pread_undo_any, ofdtx_pread_undo_any, NULL},
        {NULL, NULL,                 NULL,                 NULL},
        {NULL, NULL,                 NULL,                 NULL}};

    assert(ofdtx->type < sizeof(pread_undo)/sizeof(pread_undo[0]));
    assert(pread_undo[ofdtx->type]);

    return pread_undo[ofdtx->type][ofdtx->ccmode]();
}

