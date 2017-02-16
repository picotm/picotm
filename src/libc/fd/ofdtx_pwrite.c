/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
#include "table.h"
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
#include "fdtx.h"
#include "comfd.h"

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
pwrite_wait(int fildes, const void *buf, size_t nbyte, off_t off)
{
    ticks t0 = getticks();
    ticks t1 = getticks();

    if (!nbyte) {
        while ( (t1-t0) < 765 ) {
            t1 = getticks();
        }
    } else {

        // approximation of single-thread cycles
        long limit = 1724 + (1139*nbyte)/1000;

        while ( (t1-t0) < limit ) {
            t1 = getticks();
        }
    }

    return nbyte;
}*/

static ssize_t
ofdtx_pwrite_exec_noundo(struct ofdtx *ofdtx, int fildes,
                                              const void *buf,
                                              size_t nbyte,
                                              off_t offset,
                                              int *cookie)
{
    return TEMP_FAILURE_RETRY(pwrite(fildes, buf, nbyte, offset));
}

static ssize_t
ofdtx_pwrite_exec_regular_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                  size_t nbyte, off_t offset,
                                                  int *cookie)
{
    if (__builtin_expect(!!cookie, 1)) {

        /* add data to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx, nbyte, offset, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_pwrite_exec_regular_2pl(struct ofdtx *ofdtx, int fildes, const void *buf,
                                                   size_t nbyte, off_t offset,
                                                   int *cookie)
{
    /* register written data */

    if (__builtin_expect(!!cookie, 1)) {

        int err;

        /* lock region */

        if ((err = ofdtx_2pl_lock_region(ofdtx, nbyte, offset, 1)) < 0) {
            return err;
        }

        /* add data to write set */
        if ((*cookie = ofdtx_append_to_writeset(ofdtx,
                                                nbyte,
                                                offset, buf)) < 0) {
            return *cookie;
        }
    }

    return nbyte;
}

static ssize_t
ofdtx_pwrite_exec_fifo_ts(struct ofdtx *ofdtx, int fildes, const void *buf,
                                               size_t nbyte, off_t offset,
                                               int *cookie)
{
    errno = ESPIPE;
    return ERR_SYSTEM;
}

ssize_t
ofdtx_pwrite_exec(struct ofdtx *ofdtx, int fildes, const void *buf,
                                       size_t nbyte, off_t offset,
                                       int *cookie,
                                       int noundo)
{
    static ssize_t (* const pwrite_exec[][4])(struct ofdtx*,
                                              int,
                                              const void*,
                                              size_t,
                                              off_t,
                                              int*) = {
        {ofdtx_pwrite_exec_noundo, NULL,                         NULL,                          NULL},
        {ofdtx_pwrite_exec_noundo, ofdtx_pwrite_exec_regular_ts, ofdtx_pwrite_exec_regular_2pl, NULL},
        {ofdtx_pwrite_exec_noundo, ofdtx_pwrite_exec_fifo_ts,    NULL,                          NULL},
        {ofdtx_pwrite_exec_noundo, NULL,                         NULL,                          NULL}};

    assert(ofdtx->type < sizeof(pwrite_exec)/sizeof(pwrite_exec[0]));
    assert(pwrite_exec[ofdtx->type]);

    if (noundo) {
        /* TX irrevokable */
        ofdtx->ccmode = CC_MODE_NOUNDO;
    } else {
        /* TX revokable */
        if ((ofdtx->ccmode == CC_MODE_NOUNDO)
            || !pwrite_exec[ofdtx->type][ofdtx->ccmode]) {
            return ERR_NOUNDO;
        }
    }

    return pwrite_exec[ofdtx->type][ofdtx->ccmode](ofdtx, fildes, buf, nbyte, offset, cookie);
}

/*
 * Apply
 */

static ssize_t
ofdtx_pwrite_apply_noundo(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    return 0;
}

static ssize_t
ofdtx_pwrite_apply_regular(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    assert(ofdtx);
    assert(fildes >= 0);
    assert(event || !n);

    ssize_t len = 0;

    while (n && !(len < 0)) {

        /* Combine writes to adjacent locations */

        off_t  off = ofdtx->wrtab[event->cookie].off;
        size_t nbyte = ofdtx->wrtab[event->cookie].nbyte;
        off_t  bufoff = ofdtx->wrtab[event->cookie].bufoff;

        int m = 1;

        while ((m < n)
                && (ofdtx->wrtab[event[m].cookie].off == off+nbyte)
                && (ofdtx->wrtab[event[m].cookie].bufoff == bufoff+nbyte)) {

            nbyte += ofdtx->wrtab[event[m].cookie].nbyte;

            ++m;
        }

        len = TEMP_FAILURE_RETRY(pwrite(fildes,
                                        ofdtx->wrbuf+bufoff,
                                        nbyte, off));

        n -= m;
        event += m;
    }

    return len < 0 ? len : 0;
}

ssize_t
ofdtx_pwrite_apply(struct ofdtx *ofdtx, int fildes, const struct com_fd_event *event, size_t n)
{
    static ssize_t (* const pwrite_apply[][4])(struct ofdtx*, int, const struct com_fd_event*, size_t) = {
        {ofdtx_pwrite_apply_noundo, NULL,                       NULL,                       NULL},
        {ofdtx_pwrite_apply_noundo, ofdtx_pwrite_apply_regular, ofdtx_pwrite_apply_regular, NULL},
        {ofdtx_pwrite_apply_noundo, NULL,                       NULL,                       NULL},
        {ofdtx_pwrite_apply_noundo, NULL,                       NULL,                       NULL}};

    assert(ofdtx->type < sizeof(pwrite_apply)/sizeof(pwrite_apply[0]));
    assert(pwrite_apply[ofdtx->type]);

    return pwrite_apply[ofdtx->type][ofdtx->ccmode](ofdtx, fildes, event, n);
}

/*
 * Undo
 */

static int
ofdtx_pwrite_any_undo(void)
{
    return 0;
}

int
ofdtx_pwrite_undo(struct ofdtx *ofdtx, int fildes, int cookie)
{
    static ssize_t (* const pwrite_undo[][4])(void) = {
        {NULL, NULL,                  NULL,                  NULL},
        {NULL, ofdtx_pwrite_any_undo, ofdtx_pwrite_any_undo, NULL},
        {NULL, NULL,                  NULL,                  NULL},
        {NULL, NULL,                  NULL,                  NULL}};

    assert(ofdtx->type < sizeof(pwrite_undo)/sizeof(pwrite_undo[0]));
    assert(pwrite_undo[ofdtx->type]);

    return pwrite_undo[ofdtx->type][ofdtx->ccmode]();
}

