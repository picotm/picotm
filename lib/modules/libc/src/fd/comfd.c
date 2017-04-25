/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comfd.h"
#include <assert.h>
#include <stdlib.h>
#include "openoptab.h"
#include "pipeoptab.h"
#include "range.h"

static int
intcmp(int a, int b)
{
    return (a>b) - (a<b);
}

static int
compare_int(const void *a, const void *b)
{
    assert(a);
    assert(b);

    return intcmp(*(const int*)a, *(const int*)b);
}

int
com_fd_init(struct com_fd *comfd, unsigned long module)
{
    assert(comfd);

    comfd->module = module;

    comfd->optcc = false;

    comfd->ofdtx_max_index = 0;
    comfd->fdtx_max_fildes = 0;

    comfd->eventtab = NULL;
    comfd->eventtablen = 0;
    comfd->eventtabsiz = 0;

    comfd->openoptab = NULL;
    comfd->openoptablen = 0;

    comfd->pipeoptab = NULL;
    comfd->pipeoptablen = 0;

    return 0;
}

void
com_fd_uninit(struct com_fd *comfd)
{
    struct ofdtx *ofdtx;
    struct fdtx *fdtx;

    assert(comfd);

    /* Uninit ofdtxs */

    for (ofdtx = comfd->ofdtx; ofdtx < comfd->ofdtx+comfd->ofdtx_max_index; ++ofdtx) {
        ofdtx_uninit(ofdtx);
    }

    /* Uninit fdtxs */

    for (fdtx = comfd->fdtx; fdtx < comfd->fdtx+comfd->fdtx_max_fildes; ++fdtx) {
        fdtx_uninit(fdtx);
    }

    pipeoptab_clear(&comfd->pipeoptab, &comfd->pipeoptablen);
    openoptab_clear(&comfd->openoptab, &comfd->openoptablen);

    free(comfd->eventtab);
}

void
com_fd_set_optcc(struct com_fd *comfd, int optcc)
{
    comfd->optcc = optcc;
}

int
com_fd_get_optcc(const struct com_fd *comfd)
{
    return comfd->optcc;
}

void
com_fd_set_validation_mode(struct com_fd *comfd,
                           enum picotm_libc_validation_mode val_mode)
{
    picotm_libc_set_validation_mode(val_mode);
}

enum picotm_libc_validation_mode
com_fd_get_validation_mode(const struct com_fd *comfd)
{
    return picotm_libc_get_validation_mode();
}

static int *
com_fd_get_ifd(const struct fdtx *fdtx, size_t fdtxlen, size_t *ifdlen)
{
    assert(fdtx || !fdtxlen);
    assert(ifdlen);

    int *ifd = NULL;
    *ifdlen = 0;

    while (fdtxlen) {
        --fdtxlen;

        if (fdtx_holds_ref(fdtx)) {
            void *tmp = picotm_tabresize(ifd, *ifdlen, (*ifdlen)+1, sizeof(ifd[0]));

            if (!tmp) {
                free(ifd);
                return NULL;
            }
            ifd = tmp;

            ifd[(*ifdlen)++] = fdtx->fildes;
        }

        ++fdtx;
    }

    return ifd;
}

static int *
com_fd_get_iofd(const struct fdtx *fdtx, const int *ifd, size_t ifdlen, size_t *iofdlen)
{
    int *iofd = NULL;
    *iofdlen = 0;

    while (ifdlen) {
        --ifdlen;

        void *tmp = picotm_tabresize(iofd, *iofdlen, (*iofdlen)+1, sizeof(iofd[0]));

        if (!tmp) {
            free(iofd);
            return NULL;
        }
        iofd = tmp;

        iofd[(*iofdlen)++] = fdtx[*ifd].ofd;
        ++ifd;
    }

    qsort(iofd, *iofdlen, sizeof(iofd[0]), compare_int);

    *iofdlen = picotm_tabuniq(iofd, *iofdlen, sizeof(*iofd), compare_int);

    return iofd;
}

/* Commit handler
 */

int
com_fd_lock(struct com_fd *comfd)
{
    size_t len;

    /* Lock fds */

    comfd->ifd = com_fd_get_ifd(comfd->fdtx, comfd->fdtx_max_fildes, &len);

    if (!comfd->ifd) {
        return -1;
    }

    const int *ifd = comfd->ifd;

    comfd->ifdlen = 0;

    while (ifd < comfd->ifd+len) {
        fdtx_pre_commit(comfd->fdtx+(*ifd));
        ++ifd;
        ++comfd->ifdlen;
    }

    /* Lock ofds */

    comfd->iofd = com_fd_get_iofd(comfd->fdtx, comfd->ifd, comfd->ifdlen, &len);

    const int *iofd = comfd->iofd;

    comfd->iofdlen = 0;

    while (iofd < comfd->iofd+len) {
        ofdtx_pre_commit(comfd->ofdtx+(*iofd));
        ++iofd;
        ++comfd->iofdlen;
    }

    return 0;
}

void
com_fd_unlock(struct com_fd *comfd)
{
    /* Unlock ofds */

    const int *iofd = comfd->iofd+comfd->iofdlen;

    while (iofd && (comfd->iofd < iofd)) {
        --iofd;
        ofdtx_post_commit(comfd->ofdtx+(*iofd));
    }

    free(comfd->iofd);
    comfd->iofdlen = 0;

    /* Unlock fds */

    const int *ifd = comfd->ifd+comfd->ifdlen;

    while (ifd && (comfd->ifd < ifd)) {
        --ifd;
        fdtx_post_commit(comfd->fdtx+(*ifd));
    }

    free(comfd->ifd);
    comfd->ifdlen = 0;
}

int
com_fd_validate(struct com_fd *comfd, int noundo)
{
    /* Validate fdtxs */

    struct fdtx *fdtx = comfd->fdtx;

    while (fdtx < comfd->fdtx+comfd->fdtx_max_fildes) {

        int res = fdtx_validate(fdtx);

        if (res < 0) {
            return res;
        }
        ++fdtx;
    }

    /* Validate ofdtxs */

    struct ofdtx *ofdtx = comfd->ofdtx;

    while (ofdtx < comfd->ofdtx+comfd->ofdtx_max_index) {

        int res = ofdtx_validate(ofdtx);

        if (res < 0) {
            return res;
        }
        ++ofdtx;
    }

    return 0;
}

int
com_fd_apply_event(struct com_fd *comfd, const struct event *event, size_t n)
{
    extern int com_fd_apply_close(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_open(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_pread(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_pwrite(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_lseek(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_read(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_write(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_fcntl(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_fsync(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_sync(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_dup(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_pipe(struct com_fd*, const struct com_fd_event*, size_t);
    /* Socket calls */
    extern int com_fd_apply_socket(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_listen(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_connect(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_accept(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_send(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_recv(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_shutdown(struct com_fd*, const struct com_fd_event*, size_t);
    extern int com_fd_apply_bind(struct com_fd*, const struct com_fd_event*, size_t);

    static int (* const apply_func[])(struct com_fd*, const struct com_fd_event*, size_t) = {
        com_fd_apply_close,
        com_fd_apply_open,
        com_fd_apply_pread,
        com_fd_apply_pwrite,
        com_fd_apply_lseek,
        com_fd_apply_read,
        com_fd_apply_write,
        com_fd_apply_fcntl,
        com_fd_apply_fsync,
        com_fd_apply_sync,
        com_fd_apply_dup,
        com_fd_apply_pipe,
        /* Socket calls */
        com_fd_apply_socket,
        com_fd_apply_listen,
        com_fd_apply_connect,
        com_fd_apply_accept,
        com_fd_apply_send,
        com_fd_apply_recv,
        com_fd_apply_shutdown,
        com_fd_apply_bind};

    assert(event || !n);
    assert(event->call < sizeof(apply_func)/sizeof(apply_func[0]));
    assert(event->cookie < comfd->eventtablen);

    int err = 0;

    while (n && !err) {

        // Merge a sequence of adjacent calls to the same action

        size_t m = 1;

        while ((m < n)
                && (event[m].call   == event->call)
                && (event[m].cookie == event->cookie+m)) {
            ++m;
        }

        err = apply_func[event->call](comfd, comfd->eventtab+event->cookie, m);

        n -= m;
        event += m;
    }

    return err;
}

/* Undo handlers
 */

int
com_fd_undo_event(struct com_fd *comfd, const struct event *event, size_t n)
{
    extern int com_fd_undo_close(struct com_fd*, int, int);
    extern int com_fd_undo_open(struct com_fd*, int, int);
    extern int com_fd_undo_pread(struct com_fd*, int, int);
    extern int com_fd_undo_pwrite(struct com_fd*, int, int);
    extern int com_fd_undo_lseek(struct com_fd*, int, int);
    extern int com_fd_undo_read(struct com_fd*, int, int);
    extern int com_fd_undo_write(struct com_fd*, int, int);
    extern int com_fd_undo_fcntl(struct com_fd*, int, int);
    extern int com_fd_undo_fsync(struct com_fd*, int, int);
    extern int com_fd_undo_sync(struct com_fd*, int, int);
    extern int com_fd_undo_dup(struct com_fd*, int, int);
    extern int com_fd_undo_pipe(struct com_fd*, int, int);
    /* Socket calls */
    extern int com_fd_undo_socket(struct com_fd*, int, int);
    extern int com_fd_undo_listen(struct com_fd*, int, int);
    extern int com_fd_undo_connect(struct com_fd*, int, int);
    extern int com_fd_undo_accept(struct com_fd*, int, int);
    extern int com_fd_undo_send(struct com_fd*, int, int);
    extern int com_fd_undo_recv(struct com_fd*, int, int);
    extern int com_fd_undo_shutdown(struct com_fd*, int, int);
    extern int com_fd_undo_bind(struct com_fd*, int, int);

    static int (* const undo_func[])(struct com_fd*, int, int) = {
        com_fd_undo_close,
        com_fd_undo_open,
        com_fd_undo_pread,
        com_fd_undo_pwrite,
        com_fd_undo_lseek,
        com_fd_undo_read,
        com_fd_undo_write,
        com_fd_undo_fcntl,
        com_fd_undo_fsync,
        com_fd_undo_sync,
        com_fd_undo_dup,
        com_fd_undo_pipe,
        /* Socket calls */
        com_fd_undo_socket,
        com_fd_undo_listen,
        com_fd_undo_connect,
        com_fd_undo_accept,
        com_fd_undo_send,
        com_fd_undo_recv,
        com_fd_undo_shutdown,
        com_fd_undo_bind};

    assert(event || !n);
    assert(event->call < sizeof(undo_func)/sizeof(undo_func[0]));
    assert(event->cookie < comfd->eventtablen);

    int err = 0;
    event += n;

    while (n && !err) {
        --event;
        err = undo_func[event->call](comfd,
                                     comfd->eventtab[event->cookie].fildes,
                                     comfd->eventtab[event->cookie].cookie);
        --n;
    }

    return err;
}

/* Update CC
 */

int
com_fd_updatecc(struct com_fd *comfd, int noundo)
{
    /* Update fdtxs */

    struct fdtx *fdtx = comfd->fdtx;

    while (fdtx < comfd->fdtx+comfd->fdtx_max_fildes) {

        if (fdtx_holds_ref(fdtx)) {
            int res = fdtx_updatecc(fdtx);

            if (res < 0) {
                return res;
            }
        }
        ++fdtx;
    }

    /* Update ofdtxs */

    struct ofdtx *ofdtx = comfd->ofdtx;

    while (ofdtx < comfd->ofdtx+comfd->ofdtx_max_index) {

        if (ofdtx_holds_ref(ofdtx)) {
            int res = ofdtx_updatecc(ofdtx);

            if (res < 0) {
                return res;
            }
        }
        ++ofdtx;
    }

    return 0;
}

/* Clear CC
 */

int
com_fd_clearcc(struct com_fd *comfd, int noundo)
{
    /* Clear fdtxs' CC */

    struct fdtx *fdtx = comfd->fdtx;

    while (fdtx < comfd->fdtx+comfd->fdtx_max_fildes) {

        if (fdtx_holds_ref(fdtx)) {
            int res = fdtx_clearcc(fdtx);

            if (res < 0) {
                return res;
            }
        }
        ++fdtx;
    }

    /* Clear ofdtxs' CC */

    struct ofdtx *ofdtx = comfd->ofdtx;

    while (ofdtx < comfd->ofdtx+comfd->ofdtx_max_index) {

        if (ofdtx_holds_ref(ofdtx)) {

            int res = ofdtx_clearcc(ofdtx);

            if (res < 0) {
                return res;
            }
        }
        ++ofdtx;
    }

    return 0;
}

struct fdtx *
com_fd_get_fdtx(struct com_fd *comfd, int fildes)
{
    struct fdtx *fdtx;

    assert(comfd);
    assert(fildes < MAXNUMFD);

    for (fdtx = comfd->fdtx+comfd->fdtx_max_fildes;
         fdtx < comfd->fdtx+fildes+1; ++fdtx) {

        if (fdtx_init(fdtx) < 0) {
            return NULL;
        }
    }

    comfd->fdtx_max_fildes = lmax(fildes+1, comfd->fdtx_max_fildes);

    return comfd->fdtx+fildes;
}

struct ofdtx *
com_fd_get_ofdtx(struct com_fd *comfd, int index)
{
    struct ofdtx *ofdtx;

    assert(comfd);
    assert(index < MAXNUMFD);

    for (ofdtx = comfd->ofdtx+comfd->ofdtx_max_index;
         ofdtx < comfd->ofdtx+index+1; ++ofdtx) {

        if (ofdtx_init(ofdtx) < 0) {
            return NULL;
        }
    }

    comfd->ofdtx_max_index = lmax(index+1, comfd->ofdtx_max_index);

    return comfd->ofdtx+index;
}

int
com_fd_inject(struct com_fd *comfd, enum com_fd_call call, int fildes,
                                                           int cookie)
{
    if (__builtin_expect(comfd->eventtablen >= comfd->eventtabsiz, 0)) {

        void *tmp = picotm_tabresize(comfd->eventtab,
                                    comfd->eventtabsiz,
                                    comfd->eventtabsiz+1,
                                    sizeof(comfd->eventtab[0]));
        if (!tmp) {
            return -1;
        }
        comfd->eventtab = tmp;

        ++comfd->eventtabsiz;
    }

    struct com_fd_event *event = comfd->eventtab+comfd->eventtablen;

    event->fildes = fildes;
    event->cookie = cookie;

    if (picotm_inject_event(comfd->module, call, comfd->eventtablen) < 0) {
        return -1;
    }

    return (int)comfd->eventtablen++;
}

void
com_fd_finish(struct com_fd *comfd)
{
    struct ofdtx *ofdtx;
    struct fdtx *fdtx;

    assert(comfd);

    /* Unref ofdtxs */

    for (ofdtx = comfd->ofdtx; ofdtx < comfd->ofdtx+comfd->ofdtx_max_index; ++ofdtx) {
        ofdtx_unref(ofdtx);
    }

    /* Unref fdtxs */

    for (fdtx = comfd->fdtx; fdtx < comfd->fdtx+comfd->fdtx_max_fildes; ++fdtx) {
        fdtx_unref(fdtx);
    }
}

