/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fildes_tx.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ofd.h"
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

int
fildes_tx_init(struct fildes_tx* self, unsigned long module)
{
    self->module = module;

    self->optcc = false;

    self->ofd_tx_max_index = 0;
    self->fd_tx_max_fildes = 0;

    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;

    self->openoptab = NULL;
    self->openoptablen = 0;

    self->pipeoptab = NULL;
    self->pipeoptablen = 0;

    return 0;
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
fildes_tx_set_optcc(struct fildes_tx* self, int optcc)
{
    self->optcc = optcc;
}

int
fildes_tx_get_optcc(const struct fildes_tx* self)
{
    return self->optcc;
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
                                          sizeof(ifd[0]));
            if (!tmp) {
                picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
                free(ifd);
                return NULL;
            }
            ifd = tmp;

            ifd[(*ifdlen)++] = fd_tx->fildes;
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
                                     sizeof(iofd[0]));
        if (!tmp) {
            picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
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

struct fd_tx*
get_fd_tx(struct fildes_tx* self, int fildes)
{
    for (struct fd_tx* fd_tx = self->fd_tx + self->fd_tx_max_fildes;
                       fd_tx < self->fd_tx + fildes + 1;
                     ++fd_tx) {

        if (fd_tx_init(fd_tx) < 0) {
            return NULL;
        }
    }

    self->fd_tx_max_fildes = lmax(fildes + 1, self->fd_tx_max_fildes);

    return self->fd_tx + fildes;
}

struct ofd_tx*
get_ofd_tx(struct fildes_tx* self, int index)
{
    for (struct ofd_tx* ofd_tx = self->ofd_tx + self->ofd_tx_max_index;
                        ofd_tx < self->ofd_tx + index + 1;
                      ++ofd_tx) {

        if (ofd_tx_init(ofd_tx) < 0) {
            return NULL;
        }
    }

    self->ofd_tx_max_index = lmax(index + 1, self->ofd_tx_max_index);

    return self->ofd_tx + index;
}

static int
append_cmd(struct fildes_tx* self, enum fildes_tx_cmd cmd, int fildes,
           int cookie)
{
    if (__builtin_expect(self->eventtablen >= self->eventtabsiz, 0)) {

        void* tmp = picotm_tabresize(self->eventtab,
                                     self->eventtabsiz,
                                     self->eventtabsiz + 1,
                                     sizeof(self->eventtab[0]));
        if (!tmp) {
            return -1;
        }
        self->eventtab = tmp;

        ++self->eventtabsiz;
    }

    struct fd_event* event = self->eventtab + self->eventtablen;

    event->fildes = fildes;
    event->cookie = cookie;

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_append_event(self->module, cmd, self->eventtablen, &error);
    if (picotm_error_is_set(&error)) {
        return -1;
    }

    return (int)self->eventtablen++;
}

/*
 * accept()
 */

int
fildes_tx_exec_accept(struct fildes_tx* self, int sockfd,
                      struct sockaddr* address, socklen_t* address_len)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, sockfd, 0);
    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, sockfd, 0, &optcc);
    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Accept connection */

    int connfd = TEMP_FAILURE_RETRY(accept(sockfd, address, address_len));
    if (connfd < 0) {
        return ERR_SYSTEM;
    }

    fd_tx = self->fd_tx + connfd;

    /* Reference fd_tx */

    if ( (err = fd_tx_ref(fd_tx, connfd, 0)) ) {
        if (TEMP_FAILURE_RETRY(close(connfd)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (append_cmd(self, CMD_ACCEPT, connfd, -1) < 0) {
        return -1;
    }

    return connfd;
}

static int
apply_accept(struct fildes_tx* self, const struct fd_event* event, size_t n,
             struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    return 0;
}

static int
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

    return 0;
}

/*
 * bind()
 */

int
fildes_tx_exec_bind(struct fildes_tx* self, int socket,
                    const struct sockaddr* address, socklen_t addresslen,
                    int isnoundo)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, socket);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, socket, 0);
    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, socket, 0, &optcc);
    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Bind */

    int cookie = -1;

    int res = ofd_tx_bind_exec(ofd_tx,
                               socket, address, addresslen,
                               &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0)
        && (append_cmd(self, CMD_BIND, socket, cookie) < 0)) {
        return -1;
    }

    return res;
}

static int
apply_bind(struct fildes_tx* self, const struct fd_event* event, size_t n,
           struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_bind_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }
        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_bind_undo(ofd_tx, ev->fildes, ev->cookie, error);
}

/*
 * close()
 */

int
fildes_tx_exec_close(struct fildes_tx* self, int fildes, int isnoundo)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);
    if (err) {
        return err;
    }

    /* Close */

    int cookie = -1;

    err = fd_tx_close_exec(fd_tx, fildes, &cookie, isnoundo);

    if (err < 0) {
        return err;
    }

    /* Inject event */
    if ((cookie >= 0)
         && (append_cmd(self, CMD_CLOSE, fildes, -1) < 0)) {
        return -1;
    }

    return 0;
}

static int
apply_close(struct fildes_tx* self, const struct fd_event* event, size_t n,
            struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        int res = fd_tx_close_apply(fd_tx, event->fildes, event->cookie, error);
        if (res < 0) {
            return -1;
        }

        --n;
        ++event;
    }

    return 0;
}

static int
undo_close(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    int res = fd_tx_close_undo(fd_tx, fildes, cookie, error);
    if (res < 0) {
        return -1;
    }

    return 0;
}

/*
 * connect()
 */

int
fildes_tx_exec_connect(struct fildes_tx* self, int sockfd,
                       const struct sockaddr* serv_addr, socklen_t addrlen,
                       int isnoundo)
{
    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, sockfd, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Connect */

    int cookie = -1;

    int res = ofd_tx_connect_exec(ofd_tx,
                                  sockfd, serv_addr, addrlen,
                                  &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_CONNECT, sockfd, cookie) < 0)) {
        return -1;
    }

    return res;
}

static int
apply_connect(struct fildes_tx* self, const struct fd_event* event, size_t n,
              struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_connect_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_connect_undo(ofd_tx, fildes, cookie, error);
}

/*
 * dup()
 */

int
fildes_tx_exec_dup(struct fildes_tx* self, int fildes, int cloexec)
{
    assert(self);

    /* Reference/validate fd_tx for fildes */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

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

    struct fd_tx* fd_tx2 = get_fd_tx(self, fildes2);
    assert(fd_tx2);

    /* Reference fd_tx for fildes2 */

    err = fd_tx_ref(fd_tx2, fildes2, 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (append_cmd(self, CMD_DUP, fildes2, -1) < 0) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return fildes2;
}

static int
apply_dup(struct fildes_tx* self, const struct fd_event* event, size_t n,
          struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    return 0;
}

static int
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

    return 0;
}

/*
 * fcntl()
 */

int
fildes_tx_exec_fcntl(struct fildes_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

    if (err) {
        return err;
    }

    /* Fcntl */

    int cookie = -1;

    int res = fd_tx_fcntl_exec(fd_tx, cmd, arg, &cookie, isnoundo);

    if (res < 0) {
        if (res == ERR_DOMAIN) {
            /* Do nothing */
        } else {
            return res;
        }
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Fcntl */

    res = ofd_tx_fcntl_exec(ofd_tx, fildes, cmd, arg, &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_FCNTL, fildes, cookie) < 0)) {
        return -1;
    }

    return res;
}

static int
apply_fcntl(struct fildes_tx* self, const struct fd_event* event, size_t n,
            struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        int res = fd_tx_fcntl_apply(fd_tx, event->cookie, error);

        if (res == ERR_DOMAIN)  {

            struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
            assert(ofd_tx);

            res = ofd_tx_fcntl_apply(ofd_tx, event->fildes, event, 1, error);
        }

        if (res < 0) {
            return -1;
        }

        --n;
        ++event;
    }

    return 0;
}

static int
undo_fcntl(struct fildes_tx* self, int fildes, int cookie,
           struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);
    assert(fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    int res = fd_tx_fcntl_undo(fd_tx, cookie, error);

    if (res == ERR_DOMAIN)  {

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        res = ofd_tx_fcntl_undo(ofd_tx, fildes, cookie, error);
    }

    return res;
}

/*
 * fsync()
 */

int
fildes_tx_exec_fsync(struct fildes_tx* self, int fildes, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Fsync */

    int cookie = -1;

    int res = ofd_tx_fsync_exec(ofd_tx, fildes, isnoundo, &cookie);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_FSYNC, fildes, fildes) < 0)) {
        return -1;
    }

    return res;
}

static int
apply_fsync(struct fildes_tx* self, const struct fd_event* event, size_t n,
            struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_fsync_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_fsync_undo(ofd_tx, fildes, cookie, error);
}

/*
 * listen()
 */

int
fildes_tx_exec_listen(struct fildes_tx* self, int sockfd, int backlog,
                      int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, sockfd, 0, &optcc);

    if (err < 0) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Connect */

    int cookie = -1;

    int res = ofd_tx_listen_exec(ofd_tx, sockfd, backlog, &cookie, isnoundo);

    if (res < 0) {
        return res;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_LISTEN, sockfd, cookie) < 0)) {
        return -1;
    }

    return res;
}

int
apply_listen(struct fildes_tx* self, const struct fd_event* event, size_t n,
             struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_listen_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

int
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

    return ofd_tx_listen_undo(ofd_tx, fildes, cookie, error);
}

/*
 * lseek()
 */

off_t
fildes_tx_exec_lseek(struct fildes_tx* self, int fildes, off_t offset,
                     int whence, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Seek */

    int cookie = -1;

    off_t pos = ofd_tx_lseek_exec(ofd_tx,
                                  fildes, offset, whence,
                                  &cookie, isnoundo);

    if ((long)pos < 0) {
        return pos;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_LSEEK, fildes, cookie) < 0)) {
        return -1;
    }

    return pos;
}

int
apply_lseek(struct fildes_tx* self, const struct fd_event* event, size_t n,
            struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_lseek_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

int
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

    return ofd_tx_lseek_undo(ofd_tx, fildes, cookie, error);
}

/*
 * open()
 */

#define DO_UNLINK(mode_) \
    ( ( (mode_)&(O_CREAT|O_EXCL) ) == (O_CREAT|O_EXCL) )

int
fildes_tx_exec_open(struct fildes_tx* self, const char* path, int oflag,
                    mode_t mode, int isnoundo)
{
    /* O_TRUNC needs irrevocability */

    if ((mode&O_TRUNC) && !isnoundo) {
        return ERR_NOUNDO;
    }

    /* Open file */

    int fildes = TEMP_FAILURE_RETRY(
        openat(vfs_module_getcwd_fildes(), path, oflag, mode));

    if (fildes < 0) {
        return ERR_SYSTEM;
    }

    /* FIXME: Distinguish between open calls. Ofd only knows one file
              description, but each open creates a new open file description;
              File position might be wrong
              Ideas: Maybe introduce open index (0: outside of Tx,
              n>0 inside Tx), or maybe reset file position on commiting open */

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref(fd_tx, fildes, OFD_FL_WANTNEW);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
            perror("close");
        }
        return err;
    }

    int cookie = openoptab_append(&self->openoptab,
                                  &self->openoptablen, DO_UNLINK(mode));

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_OPEN, fildes, cookie) < 0)) {
        if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return fildes;
}

static int
apply_open(struct fildes_tx* self, const struct fd_event* event, size_t n,
           struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    return 0;
}

static int
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

    return 0;
}

/*
 * pipe()
 */

int
fildes_tx_exec_pipe(struct fildes_tx* self, int pipefd[2])
{
    assert(self);

    /* Create pipe */

    if (TEMP_FAILURE_RETRY(pipe(pipefd)) < 0) {
        return ERR_SYSTEM;
    }

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, pipefd[0]);
    assert(fd_tx);

    enum error_code err = fd_tx_ref(fd_tx, pipefd[0], 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return err;
    }

    fd_tx = get_fd_tx(self, pipefd[1]);
    assert(fd_tx);

    err = fd_tx_ref(fd_tx, pipefd[1], 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return err;
    }

    int cookie = pipeoptab_append(&self->pipeoptab,
                                  &self->pipeoptablen, pipefd);

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_PIPE, 0, cookie) < 0)) {
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

static int
apply_pipe(struct fildes_tx* self, const struct fd_event* event, size_t n,
           struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    return 0;
}

static int
undo_pipe(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    assert(self);

    const struct pipeop* pipeop = self->pipeoptab+cookie;

    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[0])) < 0) {
        picotm_error_set_errno(error, errno);
        return ERR_SYSTEM;
    }
    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[1])) < 0) {
        picotm_error_set_errno(error, errno);
        return ERR_SYSTEM;
    }

    return 0;
}

/*
 * pread()
 */

ssize_t
fildes_tx_exec_pread(struct fildes_tx* self, int fildes, void* buf,
                     size_t nbyte, off_t off, int isnoundo)
{
    assert(self);

    /* update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

    if (err) {
        return err;
    }

    /* update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* pread */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = ofd_tx_pread_exec(ofd_tx,
                                    fildes, buf, nbyte, off,
                                    &cookie, isnoundo, val_mode);

    if (len < 0) {
        return len;
    }

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    /* possibly validate optimistic domain */
    if (ofd_tx_is_optimistic(ofd_tx)
        && (val_mode == PICOTM_LIBC_VALIDATE_DOMAIN)
        && ((err = ofd_tx_validate(ofd_tx, &error)) < 0)) {
        return err;
    }

    /* inject event */
    if ((cookie >= 0) &&
        ((err = append_cmd(self, CMD_PREAD, fildes, cookie)) < 0)) {
        return err;
    }

    return len;
}

static int
apply_pread(struct fildes_tx* self, const struct fd_event* event, size_t n,
            struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_pread_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_pread_undo(ofd_tx, fildes, cookie, error);
}

/*
 * pwrite()
 */

ssize_t
fildes_tx_exec_pwrite(struct fildes_tx* self, int fildes, const void* buf,
                      size_t nbyte, off_t off, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Pwrite */

    int cookie = -1;

    ssize_t len = ofd_tx_pwrite_exec(self->ofd_tx + fd_tx->ofd,
                                     fildes, buf, nbyte, off,
                                     &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_PWRITE, fildes, cookie) < 0)) {
        return -1;
    }

    return len;
}

static int
apply_pwrite(struct fildes_tx* self, const struct fd_event* event, size_t n,
             struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_pwrite_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_pwrite_undo(ofd_tx, fildes, cookie, error);
}

/*
 * read()
 */

ssize_t
fildes_tx_exec_read(struct fildes_tx* self, int fildes, void* buf,
                    size_t nbyte, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Read */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = ofd_tx_read_exec(ofd_tx,
                                   fildes, buf, nbyte,
                                   &cookie, isnoundo, val_mode);

    if (len < 0) {
        return len;
    }

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    /* possibly validate optimistic domain */
    if (ofd_tx_is_optimistic(ofd_tx)
        && (val_mode == PICOTM_LIBC_VALIDATE_DOMAIN)
        && ((err = ofd_tx_validate(ofd_tx, &error)) < 0)) {
        return err;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_READ, fildes, cookie) < 0)) {
        return -1;
    }

    return len;
}

static int
apply_read(struct fildes_tx* self, const struct fd_event* event, size_t n,
           struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_read_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }
        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_read_undo(ofd_tx, fildes, cookie, error) == (off_t)-1 ? -1 : 0;
}

/*
 * recv()
 */

ssize_t
fildes_tx_exec_recv(struct fildes_tx* self, int sockfd, void* buffer,
                    size_t length, int flags, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, sockfd, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Receive */

    int cookie = -1;

    ssize_t len = ofd_tx_recv_exec(ofd_tx,
                                   sockfd, buffer, length, flags,
                                   &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_RECV, sockfd, cookie) < 0)) {
        return -1;
    }

    return len;
}

static int
apply_recv(struct fildes_tx* self, const struct fd_event* event, size_t n,
           struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_recv_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_recv_undo(ofd_tx, fildes, cookie, error) == (off_t)-1 ? -1 : 0;
}

/*
 * select()
 */

static enum error_code
ref_fdset(struct fildes_tx* self, int nfds, const fd_set* fdset)
{
    assert(nfds > 0);
    assert(!nfds || fdset);

    int fildes;

    for (fildes = 0; fildes < nfds; ++fildes) {
        if (FD_ISSET(fildes, fdset)) {

            /* Update/create fd_tx */

            struct fd_tx* fd_tx = get_fd_tx(self, fildes);
            assert(fd_tx);

            enum error_code err =
                fd_tx_ref_or_validate(fd_tx, fildes, 0);

            if (err) {
                return err;
            }
        }
    }

    return 0;
}

int
fildes_tx_exec_select(struct fildes_tx* self, int nfds, fd_set* readfds,
                      fd_set* writefds, fd_set* errorfds,
                      struct timeval* timeout, int isnoundo)
{
    assert(self);

    /* Ref all selected file descriptors */

    if (readfds) {
        enum error_code err = ref_fdset(self, nfds, readfds);
        if (err) {
            return err;
        }
    }
    if (writefds) {
        enum error_code err = ref_fdset(self, nfds, writefds);
        if (err) {
            return err;
        }
    }
    if (errorfds) {
        enum error_code err = ref_fdset(self, nfds, errorfds);
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
fildes_tx_exec_send(struct fildes_tx* self, int sockfd, const void* buffer,
                    size_t length, int flags, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, sockfd, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Send */

    int cookie = -1;

    ssize_t len = ofd_tx_send_exec(ofd_tx,
                                   sockfd, buffer, length, flags,
                                   &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_SEND, sockfd, cookie) < 0)) {
        return -1;
    }

    return len;
}

static int
apply_send(struct fildes_tx* self, const struct fd_event* event, size_t n,
           struct picotm_error* error)
{
    assert(self);
    assert(event && !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_send_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_send_undo(ofd_tx, fildes, cookie, error);
}

/*
 * shutdown()
 */

int
fildes_tx_exec_shutdown(struct fildes_tx* self, int sockfd, int how,
                        int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, sockfd, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, sockfd, 0, &optcc);

    if (err < 0) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Shutdown */

    int cookie = -1;

    int len = ofd_tx_shutdown_exec(ofd_tx, sockfd, how, &cookie, isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_SHUTDOWN, sockfd, sockfd) < 0)) {
        return -1;
    }

    return len;
}

static int
apply_shutdown(struct fildes_tx* self, const struct fd_event* event, size_t n,
               struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_shutdown_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_shutdown_undo(ofd_tx, fildes, cookie, error);
}

/*
 * socket()
 */

int
fildes_tx_exec_socket(struct fildes_tx* self, int domain, int type,
                      int protocol)
{
    assert(self);

    /* Create socket */

    int sockfd = TEMP_FAILURE_RETRY(socket(domain, type, protocol));

    if (sockfd < 0) {
        return -1;
    }

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, sockfd);
    assert(fd_tx);

    enum error_code err = fd_tx_ref(fd_tx, sockfd, 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (append_cmd(self, CMD_SOCKET, sockfd, -1) < 0) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    return sockfd;
}

static int
apply_socket(struct fildes_tx* self, const struct fd_event* event, size_t n,
             struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    return 0;
}

static int
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

    return 0;
}

/*
 * sync()
 */

void
fildes_tx_exec_sync(struct fildes_tx* self)
{
    assert(self);

    /* Sync */
    sync();

    /* Inject event */
    append_cmd(self, CMD_SYNC, -1, -1);
}

static int
apply_sync(struct fildes_tx* self, const struct fd_event* event, size_t n,
           struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    sync();

    return 0;
}

static int
undo_sync(struct fildes_tx* self, int fildes, int cookie,
          struct picotm_error* error)
{
    return 0;
}

/*
 * write()
 */

ssize_t
fildes_tx_exec_write(struct fildes_tx* self, int fildes, const void* buf,
                     size_t nbyte, int isnoundo)
{
    assert(self);

    /* Update/create fd_tx */

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    enum error_code err = fd_tx_ref_or_validate(fd_tx, fildes, 0);

    if (err) {
        return err;
    }

    /* Update/create ofd_tx */

    struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
    assert(ofd_tx);

    int optcc;
    err = ofd_tx_ref(ofd_tx, fd_tx->ofd, fildes, 0, &optcc);

    if (err) {
        return err;
    }

    fildes_tx_set_optcc(self, optcc);

    /* Write */

    int cookie = -1;
    ssize_t len;

    len = ofd_tx_write_exec(ofd_tx,
                            fildes,
                            buf, nbyte, &cookie,
                            isnoundo);

    if (len < 0) {
        return len;
    }

    /* Inject event */
    if ((cookie >= 0) &&
        (append_cmd(self, CMD_WRITE, fildes, cookie) < 0)) {
        return -1;
    }

    return len;
}

static int
apply_write(struct fildes_tx* self, const struct fd_event* event, size_t n,
            struct picotm_error* error)
{
    assert(self);
    assert(event || !n);

    while (n) {

        assert(event->fildes >= 0);
        assert(event->fildes < (ssize_t)(sizeof(self->fd_tx)/sizeof(self->fd_tx[0])));

        const struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
        assert(fd_tx);

        struct ofd_tx* ofd_tx = get_ofd_tx(self, fd_tx->ofd);
        assert(ofd_tx);

        size_t m = 1;

        while ((m < n) && (event[m].fildes == event->fildes)) {
            ++m;
        }

        int res = ofd_tx_write_apply(ofd_tx, event->fildes, event, m, error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

static int
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

    return ofd_tx_write_undo(ofd_tx, fildes, cookie, error);
}

/*
 * Module interface
 */

int
fildes_tx_lock(struct fildes_tx* self, struct picotm_error* error)
{
    size_t len;

    /* Lock fds */

    self->ifd = get_ifd(self->fd_tx, self->fd_tx_max_fildes, &len, error);

    if (!self->ifd) {
        return -1;
    }

    const int* ifd = self->ifd;

    self->ifdlen = 0;

    while (ifd < self->ifd+len) {
        fd_tx_pre_commit(self->fd_tx+(*ifd));
        ++ifd;
        ++self->ifdlen;
    }

    /* Lock ofds */

    self->iofd = get_iofd(self->fd_tx, self->ifd, self->ifdlen, &len, error);

    if (!self->iofd) {
        return -1;
    }

    const int* iofd = self->iofd;

    self->iofdlen = 0;

    while (iofd < self->iofd+len) {
        ofd_tx_pre_commit(self->ofd_tx + (*iofd));
        ++iofd;
        ++self->iofdlen;
    }

    return 0;
}

void
fildes_tx_unlock(struct fildes_tx* self)
{
    /* Unlock ofds */

    const int* iofd = self->iofd+self->iofdlen;

    while (iofd && (self->iofd < iofd)) {
        --iofd;
        ofd_tx_post_commit(self->ofd_tx + (*iofd));
    }

    free(self->iofd);
    self->iofdlen = 0;

    /* Unlock fds */

    const int* ifd = self->ifd+self->ifdlen;

    while (ifd && (self->ifd < ifd)) {
        --ifd;
        fd_tx_post_commit(self->fd_tx+(*ifd));
    }

    free(self->ifd);
    self->ifdlen = 0;
}

int
fildes_tx_validate(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Validate fd_txs */

    struct fd_tx* fd_tx = self->fd_tx;

    while (fd_tx < self->fd_tx+self->fd_tx_max_fildes) {
        int res = fd_tx_validate(fd_tx, error);
        if (res < 0) {
            return res;
        }
        ++fd_tx;
    }

    /* Validate ofd_txs */

    struct ofd_tx* ofd_tx = self->ofd_tx;

    while (ofd_tx < self->ofd_tx+self->ofd_tx_max_index) {
        int res = ofd_tx_validate(ofd_tx, error);
        if (res < 0) {
            return res;
        }
        ++ofd_tx;
    }

    return 0;
}

int
fildes_tx_apply_event(struct fildes_tx* self,
                      const struct picotm_event* event, size_t n,
                      struct picotm_error* error)
{
    static int (* const apply[])(struct fildes_tx*,
                                 const struct fd_event*,
                                 size_t,
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

    while (n) {

        // Merge a sequence of adjacent calls to the same action

        size_t m = 1;

        while ((m < n)
                && (event[m].call   == event->call)
                && (event[m].cookie == event->cookie+m)) {
            ++m;
        }

        int res = apply[event->call](self, self->eventtab+event->cookie, m,
                                     error);
        if (res < 0) {
            return -1;
        }

        n -= m;
        event += m;
    }

    return 0;
}

int
fildes_tx_undo_event(struct fildes_tx* self,
                     const struct picotm_event* event, size_t n,
                     struct picotm_error* error)
{
    static int (* const undo[])(struct fildes_tx*,
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

    event += n;

    while (n) {
        --event;
        int res = undo[event->call](self,
                                    self->eventtab[event->cookie].fildes,
                                    self->eventtab[event->cookie].cookie,
                                    error);
        if (res < 0) {
            return -1;
        }

        --n;
    }

    return 0;
}

int
fildes_tx_update_cc(struct fildes_tx* self, int noundo,
                    struct picotm_error* error)
{
    /* Update fd_txs */

    struct fd_tx* fd_tx = self->fd_tx;

    while (fd_tx < self->fd_tx+self->fd_tx_max_fildes) {

        if (fd_tx_holds_ref(fd_tx)) {
            int res = fd_tx_update_cc(fd_tx, error);
            if (res < 0) {
                return res;
            }
        }
        ++fd_tx;
    }

    /* Update ofd_txs */

    struct ofd_tx* ofd_tx = self->ofd_tx;

    while (ofd_tx < self->ofd_tx+self->ofd_tx_max_index) {

        if (ofd_tx_holds_ref(ofd_tx)) {
            int res = ofd_tx_update_cc(ofd_tx, error);
            if (res < 0) {
                return res;
            }
        }
        ++ofd_tx;
    }

    return 0;
}

int
fildes_tx_clear_cc(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Clear fd_txs' CC */

    struct fd_tx* fd_tx = self->fd_tx;

    while (fd_tx < self->fd_tx+self->fd_tx_max_fildes) {

        if (fd_tx_holds_ref(fd_tx)) {
            int res = fd_tx_clear_cc(fd_tx, error);
            if (res < 0) {
                return res;
            }
        }
        ++fd_tx;
    }

    /* Clear ofd_txs' CC */

    struct ofd_tx* ofd_tx = self->ofd_tx;

    while (ofd_tx < self->ofd_tx+self->ofd_tx_max_index) {

        if (ofd_tx_holds_ref(ofd_tx)) {
            int res = ofd_tx_clear_cc(ofd_tx, error);
            if (res < 0) {
                return res;
            }
        }
        ++ofd_tx;
    }

    return 0;
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
