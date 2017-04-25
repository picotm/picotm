/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comfd.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fs/comfs.h"
#include "fs/comfstx.h"
#include "ofd.h"
#include "openop.h"
#include "openoptab.h"
#include "pipeop.h"
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

/*
 * accept()
 */

int
com_fd_exec_accept(struct com_fd *data, int sockfd, struct sockaddr *address,
                                          socklen_t *address_len)
{
    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, sockfd, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Accept connection */

    int connfd = TEMP_FAILURE_RETRY(accept(sockfd, address, address_len));

    if (connfd < 0) {
        return ERR_SYSTEM;
    }

    fdtx = data->fdtx+connfd;

    /* Reference fdtx */

    if ( (err = fdtx_ref(fdtx, connfd, 0)) ) {
        if (TEMP_FAILURE_RETRY(close(connfd)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (com_fd_inject(data, ACTION_ACCEPT, connfd, -1) < 0) {
        return -1;
    }

    return connfd;
}

int
com_fd_apply_accept(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_accept(struct com_fd *data, int fildes, int cookie)
{
    assert(data);

    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    /* Mark file descriptor to be closed */
    fdtx_signal_close(fdtx);

    return 0;
}

/*
 * bind()
 */

int
com_fd_exec_bind(struct com_fd *data, int socket, const struct sockaddr *address,
                                              socklen_t addresslen, int isnoundo)
{
    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, socket);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, socket, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, socket, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Bind */

    int cookie = -1;

    int res = ofdtx_bind_exec(ofdtx,
                              socket, address, addresslen,
                              &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0)
        && (com_fd_inject(data, ACTION_BIND, socket, cookie) < 0)) {
        return -1;
    }

    return res;
}

int
com_fd_apply_bind(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_bind_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_bind(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(cookie < data->eventtablen);

    const struct com_fd_event *ev = data->eventtab+cookie;

    assert(ev->fildes >= 0);
    assert(ev->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, ev->fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_bind_undo(ofdtx, ev->fildes, ev->cookie) < 0 ? -1 : 0;
}

/*
 * close()
 */

int
com_fd_exec_close(struct com_fd *data, int fildes, int isnoundo)
{
    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Close */

    int cookie = -1;

    err = fdtx_close_exec(fdtx, fildes, &cookie, isnoundo);

    if (err < 0) {
        return err;
    }

    /* Inject event */
    if ((cookie >= 0)
         && (com_fd_inject(data, ACTION_CLOSE, fildes, -1) < 0)) {
        return -1;
    }

    return 0;
}

int
com_fd_apply_close(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        err = fdtx_close_apply(fdtx, event->fildes, event->cookie) < 0 ? -1 : 0;
        --n;
        ++event;
    }

    return err;
}

int
com_fd_undo_close(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    return fdtx_close_undo(fdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * connect()
 */

int
com_fd_exec_connect(struct com_fd *data, int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen, int isnoundo)
{
    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, sockfd, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Connect */

    int cookie = -1;

    int res = ofdtx_connect_exec(ofdtx,
                                 sockfd, serv_addr, addrlen,
                                 &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_CONNECT, sockfd, cookie) < 0)) {
        return -1;
    }

    return res;
}

int
com_fd_apply_connect(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_connect_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_connect(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_connect_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * dup()
 */

int
com_fd_exec_dup(struct com_fd *data, int fildes, int cloexec)
{
    assert(data);

    /* Reference/validate fdtx for fildes */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Duplicate fildes */

    static const int dupcmd[] = {F_DUPFD, F_DUPFD_CLOEXEC};

    int res = TEMP_FAILURE_RETRY(fcntl(fildes, dupcmd[!!cloexec], 0));
    if (res < 0) {
        return ERR_SYSTEM;
    }
    int fildes2 = res;

    struct fdtx *fdtx2 = com_fd_get_fdtx(data, fildes2);
    assert(fdtx2);

    /* Reference fdtx for fildes2 */

    err = fdtx_ref(fdtx2, fildes2, 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (com_fd_inject(data, ACTION_DUP, fildes2, -1) < 0) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return fildes2;
}

int
com_fd_apply_dup(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_dup(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    /* Mark file descriptor to be closed. This works, because dup() occured
       inside transaction. So no other transaction should have access to it. */
    fdtx_signal_close(fdtx);

    return 0;
}

/*
 * fcntl()
 */

int
com_fd_exec_fcntl(struct com_fd *data, int fildes, int cmd, union com_fd_fcntl_arg *arg, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Fcntl */

    int cookie = -1;

    int res = fdtx_fcntl_exec(fdtx, cmd, arg, &cookie, isnoundo);

    if (res < 0) {
        if (res == ERR_DOMAIN) {
            /* Do nothing */
        } else {
            return res;
        }
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Fcntl */

    res = ofdtx_fcntl_exec(ofdtx, fildes, cmd, arg, &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_FCNTL, fildes, cookie) < 0)) {
        return -1;
    }

    return res;
}

int
com_fd_apply_fcntl(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        int res = fdtx_fcntl_apply(fdtx, event->cookie);

        if (res == ERR_DOMAIN)  {

            struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
            assert(ofdtx);

            res = ofdtx_fcntl_apply(ofdtx, event->fildes, event, 1);
        }

        err = res < 0 ? -1 : 0;
        --n;
        ++event;
    }

    return err;
}

int
com_fd_undo_fcntl(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    int res = fdtx_fcntl_undo(fdtx, cookie);

    if (res == ERR_DOMAIN)  {

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        res = ofdtx_fcntl_undo(ofdtx, fildes, cookie);
    }

    return res < 0 ? -1 : 0;
}

/*
 * fcntl()
 */

int
com_fd_exec_fsync(struct com_fd *data, int fildes, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Fsync */

    int cookie = -1;

    int res = ofdtx_fsync_exec(ofdtx, fildes, isnoundo, &cookie);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_FSYNC, fildes, fildes) < 0)) {
        return -1;
    }

    return res;
}

int
com_fd_apply_fsync(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_fsync_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_fsync(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_fsync_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * listen()
 */

int
com_fd_exec_listen(struct com_fd *data, int sockfd, int backlog, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, sockfd, 0, &optcc);

    if (err < 0) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Connect */

    int cookie = -1;

    int res = ofdtx_listen_exec(ofdtx, sockfd, backlog, &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_LISTEN, sockfd, cookie) < 0)) {
        return -1;
    }

    return res;
}

int
com_fd_apply_listen(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_listen_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_listen(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_listen_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * lseek()
 */

off_t
com_fd_exec_lseek(struct com_fd *data, int fildes, off_t offset, int whence, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Seek */

    int cookie = -1;

    off_t pos = ofdtx_lseek_exec(ofdtx,
                                 fildes, offset, whence,
                                 &cookie, isnoundo);

    if ((long)pos < 0) {
        return pos;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_LSEEK, fildes, cookie) < 0)) {
        return -1;
    }

    return pos;
}

int
com_fd_apply_lseek(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_lseek_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_lseek(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_lseek_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * open()
 */

#define DO_UNLINK(mode_) \
    ( ( (mode_)&(O_CREAT|O_EXCL) ) == (O_CREAT|O_EXCL) )

int
com_fd_exec_open(struct com_fd *data, const char *path, int oflag,
                                                        mode_t mode,
                                                        int isnoundo)
{
    struct com_fs *fsdata = com_fs_tx_aquire_data();
    assert(fsdata);

    /* O_TRUNC needs irrevocability */

    if ((mode&O_TRUNC) && !isnoundo) {
        return ERR_NOUNDO;
    }

    /* Open file */

    int fildes =
        TEMP_FAILURE_RETRY(openat(com_fs_get_cwd(fsdata), path, oflag, mode));

    if (fildes < 0) {
        return ERR_SYSTEM;
    }

    /* FIXME: Distinguish between open calls. Ofd only knows one file
              description, but each open creates a new open file description;
              File position might be wrong
              Ideas: Maybe introduce open index (0: outside of Tx,
              n>0 inside Tx), or maybe reset file position on commiting open */

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref(fdtx, fildes, OFD_FL_WANTNEW);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
            perror("close");
        }
        return err;
    }

    int cookie = openoptab_append(&data->openoptab,
                                  &data->openoptablen, DO_UNLINK(mode));

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_OPEN, fildes, cookie) < 0)) {
        if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return fildes;
}

int
com_fd_apply_open(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_open(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);
    assert(cookie < data->openoptablen);

    if (data->openoptab[cookie].unlink) {

        char path[64];

        sprintf(path, "/proc/self/fd/%d", fildes);

        char *canonpath = canonicalize_file_name(path);

        if (canonpath) {

            struct stat buf[2];

            if (fstat(fildes, buf+0) != -1
                && stat(canonpath, buf+1) != -1
                && buf[0].st_dev == buf[1].st_dev
                && buf[0].st_ino == buf[1].st_ino) {

                if (unlink(canonpath) < 0) {
                    perror("unlink");
                }
            }

            free(canonpath);
        } else {
            perror("canonicalize_file_name");
        }
    }

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    /* Mark file descriptor to be closed */
    fdtx_signal_close(fdtx);

    return 0;
}

/*
 * pipe()
 */

int
com_fd_exec_pipe(struct com_fd *data, int pipefd[2])
{
    assert(data);

    /* Create pipe */

    if (TEMP_FAILURE_RETRY(pipe(pipefd)) < 0) {
        return ERR_SYSTEM;
    }

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, pipefd[0]);
    assert(fdtx);

    enum error_code err = fdtx_ref(fdtx, pipefd[0], 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return err;
    }

    fdtx = com_fd_get_fdtx(data, pipefd[1]);
    assert(fdtx);

    err = fdtx_ref(fdtx, pipefd[1], 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return err;
    }

    int cookie = pipeoptab_append(&data->pipeoptab,
                                  &data->pipeoptablen, pipefd);

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_PIPE, 0, cookie) < 0)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return 0;
}

int
com_fd_apply_pipe(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_pipe(struct com_fd *data, int fildes, int cookie)
{
    assert(data);

    const struct pipeop *pipeop = data->pipeoptab+cookie;

    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[0])) < 0) {
        return ERR_SYSTEM;
    }
    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[1])) < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

/*
 * pread()
 */

ssize_t
com_fd_exec_pread(struct com_fd *data, int fildes, void *buf, size_t nbyte, off_t off, int isnoundo)
{
    assert(data);

    /* update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* pread */

    enum picotm_libc_validation_mode val_mode = com_fd_get_validation_mode(data);

    int cookie = -1;

    ssize_t len = ofdtx_pread_exec(ofdtx,
                                   fildes, buf, nbyte, off,
                                   &cookie, isnoundo, val_mode);

    if (len < 0) {
        return len;
    }

    /* possibly validate optimistic domain */
    if (ofdtx_is_optimistic(ofdtx)
        && (val_mode == PICOTM_LIBC_VALIDATE_DOMAIN)
        && ((err = ofdtx_validate(ofdtx)) < 0)) {
        return err;
    }

    /* inject event */
    if ((cookie >= 0) &&
        ((err = com_fd_inject(data, ACTION_PREAD, fildes, cookie)) < 0)) {
        return err;
    }

    return len;
}

int
com_fd_apply_pread(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_pread_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_pread(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_pread_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * pwrite()
 */

ssize_t
com_fd_exec_pwrite(struct com_fd *data, int fildes, const void *buf, size_t nbyte, off_t off, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Pwrite */

    int cookie = -1;

    ssize_t len = ofdtx_pwrite_exec(data->ofdtx+fdtx->ofd,
                                    fildes, buf, nbyte, off,
                                    &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_PWRITE, fildes, cookie) < 0)) {
        return -1;
    }

    return len;
}

int
com_fd_apply_pwrite(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_pwrite_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_pwrite(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_pwrite_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * read()
 */

ssize_t
com_fd_exec_read(struct com_fd *data, int fildes, void *buf, size_t nbyte, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Read */

    enum picotm_libc_validation_mode val_mode = com_fd_get_validation_mode(data);

    int cookie = -1;

    ssize_t len = ofdtx_read_exec(ofdtx,
                                  fildes, buf, nbyte,
                                  &cookie, isnoundo, val_mode);

    if (len < 0) {
        return len;
    }

    /* possibly validate optimistic domain */
    if (ofdtx_is_optimistic(ofdtx)
        && (val_mode == PICOTM_LIBC_VALIDATE_DOMAIN)
        && ((err = ofdtx_validate(ofdtx)) < 0)) {
        return err;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_READ, fildes, cookie) < 0)) {
        return -1;
    }

    return len;
}

int
com_fd_apply_read(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_read_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_read(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_read_undo(ofdtx, fildes, cookie) == (off_t)-1 ? -1 : 0;
}

/*
 * recv()
 */

ssize_t
com_fd_exec_recv(struct com_fd *data, int sockfd, void *buffer, size_t length, int flags, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, sockfd, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Receive */

    int cookie = -1;

    ssize_t len = ofdtx_recv_exec(ofdtx,
                                  sockfd, buffer, length, flags,
                                 &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_RECV, sockfd, cookie) < 0)) {
        return -1;
    }

    return len;
}

int
com_fd_apply_recv(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_recv_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_recv(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_recv_undo(ofdtx, fildes, cookie) == (off_t)-1 ? -1 : 0;
}

/*
 * select()
 */

static enum error_code
ref_fdset(struct com_fd *data, int nfds, const fd_set *fdset)
{
    assert(nfds > 0);
    assert(!nfds || fdset);

    int fildes;

    for (fildes = 0; fildes < nfds; ++fildes) {
        if (FD_ISSET(fildes, fdset)) {

            /* Update/create fdtx */

            struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
            assert(fdtx);

            enum error_code err =
                fdtx_ref_or_validate(fdtx, fildes, 0);

            if (err) {
                return err;
            }
        }
    }

    return 0;
}

int
com_fd_exec_select(struct com_fd *data, int nfds, fd_set *readfds,
                                                  fd_set *writefds,
                                                  fd_set *errorfds,
                                                  struct timeval *timeout,
                                                  int isnoundo)
{
    assert(data);

    /* Ref all selected file descriptors */

    if (readfds) {
        enum error_code err = ref_fdset(data, nfds, readfds);
        if (err) {
            return err;
        }
    }
    if (writefds) {
        enum error_code err = ref_fdset(data, nfds, writefds);
        if (err) {
            return err;
        }
    }
    if (errorfds) {
        enum error_code err = ref_fdset(data, nfds, errorfds);
        if (err) {
            return err;
        }
    }

    int res;

    if (!timeout && !isnoundo) {

        /* Arbitrarily choosen default timeout of 5 sec */
        struct timeval def_timeout = {5, 0};

        res = select(nfds, readfds, writefds, errorfds, &def_timeout);

        if (!res) {
            return ERR_CONFLICT;
        }
    } else {
        res = select(nfds, readfds, writefds, errorfds, timeout);
    }

    return res;
}

/*
 * send()
 */

ssize_t
com_fd_exec_send(struct com_fd *data, int sockfd, const void *buffer, size_t length, int flags, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, sockfd, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Send */

    int cookie = -1;

    ssize_t len = ofdtx_send_exec(ofdtx,
                                  sockfd, buffer, length, flags,
                                 &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_SEND, sockfd, cookie) < 0)) {
        return -1;
    }

    return len;
}

int
com_fd_apply_send(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event && !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_send_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_send(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_send_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * shutdown()
 */

int
com_fd_exec_shutdown(struct com_fd *data, int sockfd, int how, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, sockfd, 0, &optcc);

    if (err < 0) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Shutdown */

    int cookie = -1;

    int len = ofdtx_shutdown_exec(ofdtx, sockfd, how, &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_SHUTDOWN, sockfd, sockfd) < 0)) {
        return -1;
    }

    return len;
}

int
com_fd_apply_shutdown(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_shutdown_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_shutdown(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_shutdown_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}

/*
 * socket()
 */

int
com_fd_exec_socket(struct com_fd *data, int domain, int type, int protocol)
{
    assert(data);

    /* Create socket */

    int sockfd = TEMP_FAILURE_RETRY(socket(domain, type, protocol));

    if (sockfd < 0) {
        return -1;
    }

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref(fdtx, sockfd, 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (com_fd_inject(data, ACTION_SOCKET, sockfd, -1) < 0) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    return sockfd;
}

int
com_fd_apply_socket(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_socket(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    /* Mark file descriptor to be closed. This works, because dup() occured
       inside transaction. So no other transaction should have access to it. */
    fdtx_signal_close(fdtx);

    return 0;
}

/*
 * sync()
 */

void
com_fd_exec_sync(struct com_fd *data)
{
    assert(data);

    /* Sync */
    sync();

    /* Inject event */
    com_fd_inject(data, ACTION_SYNC, -1, -1);
}

int
com_fd_apply_sync(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    sync();

    return 0;
}

int
com_fd_undo_sync(struct com_fd *data, int fildes, int cookie)
{
    return 0;
}

/*
 * write()
 */

ssize_t
com_fd_exec_write(struct com_fd *data, int fildes, const void *buf, size_t nbyte, int isnoundo)
{
    assert(data);

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofdtx */

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    int optcc;
    err = ofdtx_ref(ofdtx, fdtx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    com_fd_set_optcc(data, optcc);

    /* Write */

    int cookie = -1;
    ssize_t len;

    len = ofdtx_write_exec(ofdtx,
                           fildes,
                           buf, nbyte, &cookie,
                           isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_WRITE, fildes, cookie) < 0)) {
        return -1;
    }

    return len;
}

int
com_fd_apply_write(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    int err = 0;

    while (n && !err) {

        assert(event->fildes >= 0);
        assert(event->fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

        const struct fdtx *fdtx = com_fd_get_fdtx(data, event->fildes);
        assert(fdtx);

        struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
        assert(ofdtx);

        int m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        err = ofdtx_write_apply(ofdtx, event->fildes, event, m) < 0 ? -1 : 0;
        n -= m;
        event += m;
    }

    return err;
}

int
com_fd_undo_write(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < sizeof(data->fdtx)/sizeof(data->fdtx[0]));

    const struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    struct ofdtx *ofdtx = com_fd_get_ofdtx(data, fdtx->ofd);
    assert(ofdtx);

    return ofdtx_write_undo(ofdtx, fildes, cookie) < 0 ? -1 : 0;
}
