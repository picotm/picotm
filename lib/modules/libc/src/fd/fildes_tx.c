/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fildes_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-lib-tab.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fdtab.h"
#include "ofd.h"
#include "ofdtab.h"
#include "openop.h"
#include "openoptab.h"
#include "pipeop.h"
#include "pipeoptab.h"
#include "range.h"
#include "vfs/module.h"

enum fildes_tx_cmd {
    CMD_CLOSE = 0,
    CMD_OPEN,
    CMD_PREAD,
    CMD_PWRITE,
    CMD_LSEEK,
    CMD_READ,
    CMD_WRITE,
    CMD_FCNTL,
    CMD_FSYNC,
    CMD_SYNC,
    CMD_DUP,
    CMD_PIPE,
    /* Socket calls */
    CMD_SOCKET,
    CMD_LISTEN,
    CMD_CONNECT,
    CMD_ACCEPT,
    CMD_SEND,
    CMD_RECV,
    CMD_SHUTDOWN,
    CMD_BIND
};

static int
intcmp(int a, int b)
{
    return (a > b) - (a < b);
}

static int
compare_int(const void* a, const void* b)
{
    assert(a);
    assert(b);

    return intcmp(*(const int*)a, *(const int*)b);
}

void
fildes_tx_init(struct fildes_tx* self, unsigned long module)
{
    self->module = module;

    self->ofd_tx_max_index = 0;
    self->fd_tx_max_fildes = 0;

    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;

    self->openoptab = NULL;
    self->openoptablen = 0;

    self->pipeoptab = NULL;
    self->pipeoptablen = 0;
}

void
fildes_tx_uninit(struct fildes_tx* self)
{
    /* Uninit ofd_txs */

    for (struct ofd_tx* ofd_tx = self->ofd_tx;
                        ofd_tx < self->ofd_tx + self->ofd_tx_max_index;
                      ++ofd_tx) {
        ofd_tx_uninit(ofd_tx);
    }

    /* Uninit fd_txs */

    for (struct fd_tx* fd_tx = self->fd_tx;
                       fd_tx < self->fd_tx + self->fd_tx_max_fildes;
                     ++fd_tx) {
        fd_tx_uninit(fd_tx);
    }

    pipeoptab_clear(&self->pipeoptab, &self->pipeoptablen);
    openoptab_clear(&self->openoptab, &self->openoptablen);

    free(self->eventtab);
}

void
fildes_tx_set_validation_mode(struct fildes_tx* self,
                              enum picotm_libc_validation_mode val_mode)
{
    picotm_libc_set_validation_mode(val_mode);
}

enum picotm_libc_validation_mode
fildes_tx_get_validation_mode(const struct fildes_tx* self)
{
    return picotm_libc_get_validation_mode();
}

static int*
get_ifd(const struct fd_tx* fd_tx, size_t fd_txlen, size_t* ifdlen,
        struct picotm_error* error)
{
    int* ifd = NULL;
    *ifdlen = 0;

    while (fd_txlen) {
        --fd_txlen;

        if (fd_tx_holds_ref(fd_tx)) {
            void* tmp = picotm_tabresize(ifd, *ifdlen, (*ifdlen) + 1,
                                         sizeof(ifd[0]), error);
            if (picotm_error_is_set(error)) {
                free(ifd);
                return NULL;
            }
            ifd = tmp;

            ifd[(*ifdlen)++] = fd_tx->fd - fdtab;
        }

        ++fd_tx;
    }

    return ifd;
}

static int*
get_iofd(const struct fd_tx* fd_tx, const int* ifd, size_t ifdlen,
         size_t* iofdlen, struct picotm_error* error)
{
    int* iofd = NULL;
    *iofdlen = 0;

    while (ifdlen) {
        --ifdlen;

        void* tmp = picotm_tabresize(iofd, *iofdlen, (*iofdlen) + 1,
                                     sizeof(iofd[0]), error);
        if (picotm_error_is_set(error)) {
            free(iofd);
            return NULL;
        }
        iofd = tmp;

        iofd[(*iofdlen)++] = fd_tx[*ifd].ofd;
        ++ifd;
    }

    qsort(iofd,* iofdlen, sizeof(iofd[0]), compare_int);

    *iofdlen = picotm_tabuniq(iofd, *iofdlen, sizeof(*iofd), compare_int);

    return iofd;
}

static struct fd_tx*
get_fd_tx(struct fildes_tx* self, int fildes)
{
    for (struct fd_tx* fd_tx = self->fd_tx + self->fd_tx_max_fildes;
                       fd_tx < self->fd_tx + fildes + 1;
                     ++fd_tx) {

        fd_tx_init(fd_tx);
    }

    self->fd_tx_max_fildes = lmax(fildes + 1, self->fd_tx_max_fildes);

    return self->fd_tx + fildes;
}

static struct fd_tx*
get_fd_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                   struct picotm_error* error)
{
    struct fd_tx* fd_tx = get_fd_tx(self, fildes);

    if (fd_tx_holds_ref(fd_tx)) {

        /* Validate reference or return error if fd has been closed */
        fd_tx_lock(fd_tx);
        fd_tx_validate(fd_tx, error);
        if (picotm_error_is_set(error)) {
            fd_tx_unlock(fd_tx);
            return NULL;
        }
        fd_tx_unlock(fd_tx);

        return fd_tx;
    }

    fd_tx_ref(fd_tx, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd_tx;
}

static struct ofd_tx*
get_ofd_tx(struct fildes_tx* self, int index)
{
    for (struct ofd_tx* ofd_tx = self->ofd_tx + self->ofd_tx_max_index;
                        ofd_tx < self->ofd_tx + index + 1;
                      ++ofd_tx) {
        ofd_tx_init(ofd_tx);
    }

    self->ofd_tx_max_index = lmax(index + 1, self->ofd_tx_max_index);

    return self->ofd_tx + index;
}

static struct ofd_tx*
get_ofd_tx_with_ref(struct fildes_tx* self, int index, struct ofd* ofd,
                    int fildes, unsigned long flags,
                    struct picotm_error* error)
{
    struct ofd_tx* ofd_tx = get_ofd_tx(self, index);

    if (ofd_tx_holds_ref(ofd_tx)) {
        return ofd_tx;
    }

    ofd_tx_ref(ofd_tx, ofd, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return ofd_tx;
}

static int
append_cmd(struct fildes_tx* self, enum fildes_tx_cmd cmd, int fildes,
           int cookie, struct picotm_error* error)
{
    if (__builtin_expect(self->eventtablen >= self->eventtabsiz, 0)) {

        void* tmp = picotm_tabresize(self->eventtab,
                                     self->eventtabsiz,
                                     self->eventtabsiz + 1,
                                     sizeof(self->eventtab[0]), error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        self->eventtab = tmp;

        ++self->eventtabsiz;
    }

    struct fd_event* event = self->eventtab + self->eventtablen;

    event->fildes = fildes;
    event->cookie = cookie;

    picotm_append_event(self->module, cmd, self->eventtablen, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return (int)self->eventtablen++;
}

/*
 * accept()
 */

int
fildes_tx_exec_accept(struct fildes_tx* self, int sockfd,
                      struct sockaddr* address, socklen_t* address_len,
                      struct picotm_error* error)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    get_ofd_tx_with_ref(self, fd_tx->ofd, ofdtab + fd_tx->ofd, sockfd, 0,
                        error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Accept connection */

    int connfd = TEMP_FAILURE_RETRY(accept(sockfd, address, address_len));
    if (connfd < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    fd_tx = self->fd_tx + connfd;

    /* Reference fd_tx */

    fd_tx_ref(fd_tx, connfd, 0, error);
    if (picotm_error_is_set(error)) {
        goto err_fd_tx_ref;
    }

    /* Inject event */
    append_cmd(self, CMD_ACCEPT, connfd, -1, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return connfd;

err_fd_tx_ref:
    if (TEMP_FAILURE_RETRY(close(connfd)) < 0) {
        perror("close");
    }
    return -1;
}

static void
apply_accept(struct fildes_tx* self, const struct fd_event* event,
             struct picotm_error* error)
{
    assert(self);
    assert(event);
}

static void
undo_accept(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);

    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    /* Mark file descriptor to be closed */
    fd_tx_signal_close(fd_tx);
}

/*
 * bind()
 */

int
fildes_tx_exec_bind(struct fildes_tx* self, int socket,
                    const struct sockaddr* address, socklen_t addresslen,
                    int isnoundo, struct picotm_error* error)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, socket, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                socket, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Bind */

    int cookie = -1;

    int res = ofd_tx_bind_exec(ofd_tx,
                               socket, address, addresslen,
                               &cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_BIND, socket, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_bind(struct fildes_tx* self, const struct fd_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_bind_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_bind(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(cookie < (ssize_t)self->eventtablen);

    const struct fd_event* ev = self->eventtab+cookie;

    assert(ev->fildes >= 0);
    assert(ev->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, ev->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_bind_undo(ofd_tx, ev->fildes, ev->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * close()
 */

int
fildes_tx_exec_close(struct fildes_tx* self, int fildes, int isnoundo,
                     struct picotm_error* error)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Close */

    int cookie = -1;

    int res = fd_tx_close_exec(fd_tx, fildes, &cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_CLOSE, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_close(struct fildes_tx* self, const struct fd_event* event,
            struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_close_apply(fd_tx, event->fildes, event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_close(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_close_undo(fd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * connect()
 */

int
fildes_tx_exec_connect(struct fildes_tx* self, int sockfd,
                       const struct sockaddr* serv_addr, socklen_t addrlen,
                       int isnoundo, struct picotm_error* error)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Connect */

    int cookie = -1;

    int res = ofd_tx_connect_exec(ofd_tx,
                                  sockfd, serv_addr, addrlen,
                                  &cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_CONNECT, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_connect(struct fildes_tx* self, const struct fd_event* event,
              struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_connect_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_connect(struct fildes_tx* self, int fildes, int cookie,
             struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_connect_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * dup()
 */

int
fildes_tx_exec_dup(struct fildes_tx* self, int fildes, int cloexec,
                   struct picotm_error* error)
{
    assert(self);

    /* Reference/validate fd_tx for fildes */

    get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Duplicate fildes */

    static const int dupcmd[] = {F_DUPFD, F_DUPFD_CLOEXEC};

    int res = TEMP_FAILURE_RETRY(fcntl(fildes, dupcmd[!!cloexec], 0));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    int fildes2 = res;

    struct fd_tx* fd_tx2 = get_fd_tx(self, fildes2);
    assert(fd_tx2);

    /* Reference fd_tx for fildes2 */

    fd_tx_ref(fd_tx2, fildes2, 0, error);

    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return -1;
    }

    /* Inject event */
    append_cmd(self, CMD_DUP, fildes2, -1, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return -1;
    }

    return fildes2;
}

static void
apply_dup(struct fildes_tx* self, const struct fd_event* event,
          struct picotm_error* error)
{
    assert(self);
    assert(event);
}

static void
undo_dup(struct fildes_tx* self, int fildes, int cookie,
         struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    /* Mark file descriptor to be closed. This works, because dup() occured
       inside transaction. So no other transaction should have access to it. */
    fd_tx_signal_close(fd_tx);
}

/*
 * fcntl()
 */

int
fildes_tx_exec_fcntl(struct fildes_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, int isnoundo,
                     struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int cookie = -1;
    int res;

    switch (cmd) {
        case F_GETFD:
            /* fall through */
        case F_SETFD: {
            /* Fcntl */

            res = fd_tx_fcntl_exec(fd_tx, cmd, arg, &cookie, isnoundo, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }
            break;
        }
        default: {
            /* Update/create ofd_tx */

            struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                        ofdtab + fd_tx->ofd,
                                                        fildes, 0, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }

            /* Fcntl */

            res = ofd_tx_fcntl_exec(ofd_tx, fildes, cmd, arg, &cookie,
                                    isnoundo, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }
            break;
        }

    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_FCNTL, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_fcntl(struct fildes_tx* self, const struct fd_event* event,
            struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    bool next_domain = false;

    fd_tx_fcntl_apply(fd_tx, event->cookie, &next_domain, error);

    if (next_domain)  {

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        ofd_tx_fcntl_apply(ofd_tx, event->fildes, event, error);
    }

    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_fcntl(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    bool next_domain = false;

    fd_tx_fcntl_undo(fd_tx, cookie, &next_domain, error);

    if (next_domain)  {

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        ofd_tx_fcntl_undo(ofd_tx, fildes, cookie, error);
    }

    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * fsync()
 */

int
fildes_tx_exec_fsync(struct fildes_tx* self, int fildes, int isnoundo,
                     struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Fsync */

    int cookie = -1;

    int res = ofd_tx_fsync_exec(ofd_tx, fildes, isnoundo, &cookie, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_FSYNC, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_fsync(struct fildes_tx* self, const struct fd_event* event,
            struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_fsync_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_fsync(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_fsync_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * listen()
 */

int
fildes_tx_exec_listen(struct fildes_tx* self, int sockfd, int backlog,
                      int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Connect */

    int cookie = -1;

    int res = ofd_tx_listen_exec(ofd_tx, sockfd, backlog, &cookie, isnoundo,
                                 error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_LISTEN, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return res;
}

static void
apply_listen(struct fildes_tx* self, const struct fd_event* event,
             struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_listen_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_listen(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_listen_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * lseek()
 */

off_t
fildes_tx_exec_lseek(struct fildes_tx* self, int fildes, off_t offset,
                     int whence, int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* Seek */

    int cookie = -1;

    off_t pos = ofd_tx_lseek_exec(ofd_tx,
                                  fildes, offset, whence,
                                  &cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_LSEEK, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return (off_t)-1;
        }
    }

    return pos;
}

static void
apply_lseek(struct fildes_tx* self, const struct fd_event* event,
            struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_lseek_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_lseek(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_lseek_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * open()
 */

#define DO_UNLINK(mode_) \
    ( ( (mode_)&(O_CREAT|O_EXCL) ) == (O_CREAT|O_EXCL) )

int
fildes_tx_exec_open(struct fildes_tx* self, const char* path, int oflag,
                    mode_t mode, int isnoundo, struct picotm_error* error)
{
    /* O_TRUNC needs irrevocability */

    if ((mode&O_TRUNC) && !isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* Open file */

    int fildes = TEMP_FAILURE_RETRY(
        openat(vfs_module_getcwd_fildes(), path, oflag, mode));

    if (fildes < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* FIXME: Distinguish between open calls. Ofd only knows one file
              description, but each open creates a new open file description;
              File position might be wrong
              Ideas: Maybe introduce open index (0: outside of Tx,
              n>0 inside Tx), or maybe reset file position on commiting open */

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_ref(fd_tx, fildes, OFD_FL_WANTNEW, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
            perror("close");
        }
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    int cookie = openoptab_append(&self->openoptab,
                                  &self->openoptablen, DO_UNLINK(mode),
                                  error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_OPEN, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
                perror("close");
            }
            return -1;
        }
    }

    return fildes;
}

static void
apply_open(struct fildes_tx* self, const struct fd_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);
}

static void
undo_open(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);
    assert(cookie < (ssize_t)self->openoptablen);

    if (self->openoptab[cookie].unlink) {

        char path[64];

        sprintf(path, "/proc/self/fd/%d", fildes);

        char* canonpath = canonicalize_file_name(path);

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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    /* Mark file descriptor to be closed */
    fd_tx_signal_close(fd_tx);
}

/*
 * pipe()
 */

int
fildes_tx_exec_pipe(struct fildes_tx* self, int pipefd[2],
                    struct picotm_error* error)
{
    assert(self);

    /* Create pipe */

    int res = TEMP_FAILURE_RETRY(pipe(pipefd));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, pipefd[0]);
    assert(fd_tx);

    fd_tx_ref(fd_tx, pipefd[0], 0, error);

    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return -1;
    }

    fd_tx = get_fd_tx(self, pipefd[1]);
    assert(fd_tx);

    fd_tx_ref(fd_tx, pipefd[1], 0, error);

    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return -1;
    }

    int cookie = pipeoptab_append(&self->pipeoptab,
                                  &self->pipeoptablen, pipefd,
                                  error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_PIPE, 0, cookie, error);
        if (picotm_error_is_set(error)) {
            if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
                perror("close");
            }
            if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
                perror("close");
            }
            return -1;
        }
    }

    return 0;
}

static void
apply_pipe(struct fildes_tx* self, const struct fd_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);
}

static void
undo_pipe(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);

    const struct pipeop* pipeop = self->pipeoptab+cookie;

    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[0])) < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[1])) < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * pread()
 */

ssize_t
fildes_tx_exec_pread(struct fildes_tx* self, int fildes, void* buf,
                     size_t nbyte, off_t off, int isnoundo,
                     struct picotm_error* error)
{
    assert(self);

    /* update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* pread */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = ofd_tx_pread_exec(ofd_tx,
                                    fildes, buf, nbyte, off,
                                    &cookie, isnoundo, val_mode, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_PREAD, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_pread(struct fildes_tx* self, const struct fd_event* event,
            struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_pread_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_pread(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_pread_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * pwrite()
 */

ssize_t
fildes_tx_exec_pwrite(struct fildes_tx* self, int fildes, const void* buf,
                      size_t nbyte, off_t off, int isnoundo,
                      struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Pwrite */

    int cookie = -1;

    ssize_t len = ofd_tx_pwrite_exec(ofd_tx,
                                     fildes, buf, nbyte, off,
                                     &cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_PWRITE, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_pwrite(struct fildes_tx* self, const struct fd_event* event,
             struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_pwrite_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_pwrite(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_pwrite_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * read()
 */

ssize_t
fildes_tx_exec_read(struct fildes_tx* self, int fildes, void* buf,
                    size_t nbyte, int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Read */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = ofd_tx_read_exec(ofd_tx,
                                   fildes, buf, nbyte,
                                   &cookie, isnoundo, val_mode,
                                   error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_READ, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_read(struct fildes_tx* self, const struct fd_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_read_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_read(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_read_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * recv()
 */

ssize_t
fildes_tx_exec_recv(struct fildes_tx* self, int sockfd, void* buffer,
                    size_t length, int flags, int isnoundo,
                    struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Receive */

    int cookie = -1;

    ssize_t len = ofd_tx_recv_exec(ofd_tx,
                                   sockfd, buffer, length, flags,
                                   &cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_RECV, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_recv(struct fildes_tx* self, const struct fd_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_recv_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_recv(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_recv_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * select()
 */

static void
ref_fdset(struct fildes_tx* self, int nfds, const fd_set* fdset,
          struct picotm_error* error)
{
    assert(nfds > 0);
    assert(!nfds || fdset);

    int fildes;

    for (fildes = 0; fildes < nfds; ++fildes) {
        if (FD_ISSET(fildes, fdset)) {

            /* Update/create fd_tx */

            get_fd_tx_with_ref(self, fildes, 0, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
    }
}

int
fildes_tx_exec_select(struct fildes_tx* self, int nfds, fd_set* readfds,
                      fd_set* writefds, fd_set* errorfds,
                      struct timeval* timeout, int isnoundo,
                      struct picotm_error* error)
{
    assert(self);

    /* Ref all selected file descriptors */

    if (readfds) {
        ref_fdset(self, nfds, readfds, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }
    if (writefds) {
        ref_fdset(self, nfds, writefds, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }
    if (errorfds) {
        ref_fdset(self, nfds, errorfds, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    int res;

    if (!timeout && !isnoundo) {

        /* Arbitrarily choosen default timeout of 5 sec */
        struct timeval def_timeout = {5, 0};

        res = TEMP_FAILURE_RETRY(select(nfds, readfds, writefds, errorfds,
                                        &def_timeout));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            return -1;
        }
    } else {
        res = TEMP_FAILURE_RETRY(select(nfds, readfds, writefds, errorfds,
                                        timeout));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            return -1;
        }
    }

    return res;
}

/*
 * send()
 */

ssize_t
fildes_tx_exec_send(struct fildes_tx* self, int sockfd, const void* buffer,
                    size_t length, int flags, int isnoundo,
                    struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Send */

    int cookie = -1;

    ssize_t len = ofd_tx_send_exec(ofd_tx,
                                   sockfd, buffer, length, flags,
                                   &cookie, isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_SEND, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_send(struct fildes_tx* self, const struct fd_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_send_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_send(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_send_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * shutdown()
 */

int
fildes_tx_exec_shutdown(struct fildes_tx* self, int sockfd, int how,
                        int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Shutdown */

    int cookie = -1;

    int len = ofd_tx_shutdown_exec(ofd_tx, sockfd, how, &cookie, isnoundo,
                                   error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_SHUTDOWN, sockfd, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_shutdown(struct fildes_tx* self, const struct fd_event* event,
               struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_shutdown_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_shutdown(struct fildes_tx* self, int fildes, int cookie,
              struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_shutdown_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * socket()
 */

int
fildes_tx_exec_socket(struct fildes_tx* self, int domain, int type,
                      int protocol, struct picotm_error* error)
{
    assert(self);

    /* Create socket */

    int sockfd = TEMP_FAILURE_RETRY(socket(domain, type, protocol));
    if (sockfd < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    fd_tx_ref(fd_tx, sockfd, 0, error);

    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    /* Inject event */
    append_cmd(self, CMD_SOCKET, sockfd, -1, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    return sockfd;
}

static void
apply_socket(struct fildes_tx* self, const struct fd_event* event,
             struct picotm_error* error)
{
    assert(self);
    assert(event);
}

static void
undo_socket(struct fildes_tx* self, int fildes, int cookie,
            struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    /* Mark file descriptor to be closed. This works, because dup() occured
     * inside the transaction. So no other transaction should have access to
     * it. */
    fd_tx_signal_close(fd_tx);
}

/*
 * sync()
 */

void
fildes_tx_exec_sync(struct fildes_tx* self, struct picotm_error* error)
{
    assert(self);

    /* Sync */
    sync();

    /* Inject event */
    append_cmd(self, CMD_SYNC, -1, -1, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
apply_sync(struct fildes_tx* self, const struct fd_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);

    sync();
}

static void
undo_sync(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{ }

/*
 * write()
 */

ssize_t
fildes_tx_exec_write(struct fildes_tx* self, int fildes, const void* buf,
                     size_t nbyte, int isnoundo, struct picotm_error* error)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx_with_ref(self, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fd_tx->ofd,
                                                ofdtab + fd_tx->ofd,
                                                fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Write */

    int cookie = -1;
    ssize_t len;

    len = ofd_tx_write_exec(ofd_tx,
                            fildes,
                            buf, nbyte, &cookie,
                            isnoundo, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Inject event */
    if (cookie >= 0) {
        append_cmd(self, CMD_WRITE, fildes, cookie, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    return len;
}

static void
apply_write(struct fildes_tx* self, const struct fd_event* event,
            struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->fildes >= 0);
    assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_write_apply(ofd_tx, event->fildes, event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_write(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    const struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    ofd_tx_write_undo(ofd_tx, fildes, cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Module interface
 */

void
fildes_tx_lock(struct fildes_tx* self, struct picotm_error* error)
{
    size_t len;

    /* Lock fds */

    self->ifd = get_ifd(self->fd_tx, self->fd_tx_max_fildes, &len, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    const int* ifd = self->ifd;

    self->ifdlen = 0;

    while (ifd < self->ifd+len) {
        fd_tx_lock(self->fd_tx+(*ifd));
        ++ifd;
        ++self->ifdlen;
    }

    /* Lock ofds */

    self->iofd = get_iofd(self->fd_tx, self->ifd, self->ifdlen, &len, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    const int* iofd = self->iofd;

    self->iofdlen = 0;

    while (iofd < self->iofd+len) {
        ofd_tx_lock(self->ofd_tx + (*iofd));
        ++iofd;
        ++self->iofdlen;
    }
}

void
fildes_tx_unlock(struct fildes_tx* self)
{
    /* Unlock ofds */

    const int* iofd = self->iofd+self->iofdlen;

    while (iofd && (self->iofd < iofd)) {
        --iofd;
        ofd_tx_unlock(self->ofd_tx + (*iofd));
    }

    free(self->iofd);
    self->iofdlen = 0;

    /* Unlock fds */

    const int* ifd = self->ifd+self->ifdlen;

    while (ifd && (self->ifd < ifd)) {
        --ifd;
        fd_tx_unlock(self->fd_tx+(*ifd));
    }

    free(self->ifd);
    self->ifdlen = 0;
}

void
fildes_tx_validate(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Validate fd_txs */

    struct fd_tx* fd_tx = self->fd_tx;

    while (fd_tx < self->fd_tx+self->fd_tx_max_fildes) {
        fd_tx_validate(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        ++fd_tx;
    }

    /* Validate ofd_txs */

    struct ofd_tx* ofd_tx = self->ofd_tx;

    while (ofd_tx < self->ofd_tx+self->ofd_tx_max_index) {
        ofd_tx_validate(ofd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        ++ofd_tx;
    }
}

void
fildes_tx_apply_event(struct fildes_tx* self,
                      const struct picotm_event* event,
                      struct picotm_error* error)
{
    static void (* const apply[])(struct fildes_tx*,
                                  const struct fd_event*,
                                  struct picotm_error*) = {
        apply_close,
        apply_open,
        apply_pread,
        apply_pwrite,
        apply_lseek,
        apply_read,
        apply_write,
        apply_fcntl,
        apply_fsync,
        apply_sync,
        apply_dup,
        apply_pipe,
        /* Socket calls */
        apply_socket,
        apply_listen,
        apply_connect,
        apply_accept,
        apply_send,
        apply_recv,
        apply_shutdown,
        apply_bind
    };

    apply[event->call](self, self->eventtab + event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fildes_tx_undo_event(struct fildes_tx* self,
                     const struct picotm_event* event,
                     struct picotm_error* error)
{
    static void (* const undo[])(struct fildes_tx*,
                                 int,
                                 int,
                                 struct picotm_error*) = {
        undo_close,
        undo_open,
        undo_pread,
        undo_pwrite,
        undo_lseek,
        undo_read,
        undo_write,
        undo_fcntl,
        undo_fsync,
        undo_sync,
        undo_dup,
        undo_pipe,
        /* Socket calls */
        undo_socket,
        undo_listen,
        undo_connect,
        undo_accept,
        undo_send,
        undo_recv,
        undo_shutdown,
        undo_bind
    };

    undo[event->call](self,
                      self->eventtab[event->cookie].fildes,
                      self->eventtab[event->cookie].cookie,
                      error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fildes_tx_update_cc(struct fildes_tx* self, int noundo,
                    struct picotm_error* error)
{
    /* Update fd_txs */

    struct fd_tx* fd_tx = self->fd_tx;

    while (fd_tx < self->fd_tx+self->fd_tx_max_fildes) {

        if (fd_tx_holds_ref(fd_tx)) {
            fd_tx_update_cc(fd_tx, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        ++fd_tx;
    }

    /* Update ofd_txs */

    struct ofd_tx* ofd_tx = self->ofd_tx;

    while (ofd_tx < self->ofd_tx+self->ofd_tx_max_index) {

        if (ofd_tx_holds_ref(ofd_tx)) {
            ofd_tx_update_cc(ofd_tx, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        ++ofd_tx;
    }
}

void
fildes_tx_clear_cc(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Clear fd_txs' CC */

    struct fd_tx* fd_tx = self->fd_tx;

    while (fd_tx < self->fd_tx+self->fd_tx_max_fildes) {

        if (fd_tx_holds_ref(fd_tx)) {
            fd_tx_clear_cc(fd_tx, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        ++fd_tx;
    }

    /* Clear ofd_txs' CC */

    struct ofd_tx* ofd_tx = self->ofd_tx;

    while (ofd_tx < self->ofd_tx+self->ofd_tx_max_index) {

        if (ofd_tx_holds_ref(ofd_tx)) {
            ofd_tx_clear_cc(ofd_tx, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        ++ofd_tx;
    }
}

void
fildes_tx_finish(struct fildes_tx* self)
{
    /* Unref ofd_txs */

    for (struct ofd_tx* ofd_tx = self->ofd_tx;
                        ofd_tx < self->ofd_tx + self->ofd_tx_max_index;
                      ++ofd_tx) {
        ofd_tx_unref(ofd_tx);
    }

    /* Unref fd_txs */

    for (struct fd_tx* fd_tx = self->fd_tx;
                       fd_tx < self->fd_tx + self->fd_tx_max_fildes;
                     ++fd_tx) {
        fd_tx_unref(fd_tx);
    }
}
