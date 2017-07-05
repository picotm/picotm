/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "fildes_tx.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
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

void
fildes_tx_init(struct fildes_tx* self, unsigned long module)
{
    self->module = module;

    self->fd_tx_max_fildes = 0;
    SLIST_INIT(&self->fd_tx_active_list);

    self->fifo_tx_max_index = 0;
    SLIST_INIT(&self->fifo_tx_active_list);

    self->chrdev_tx_max_index = 0;
    SLIST_INIT(&self->chrdev_tx_active_list);

    self->regfile_tx_max_index = 0;
    SLIST_INIT(&self->regfile_tx_active_list);

    self->socket_tx_max_index = 0;
    SLIST_INIT(&self->socket_tx_active_list);

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
    /* Uninit fifo_txs */

    for (struct fifo_tx* fifo_tx = self->fifo_tx;
                         fifo_tx < self->fifo_tx + self->fifo_tx_max_index;
                       ++fifo_tx) {
        fifo_tx_uninit(fifo_tx);
    }

    /* Uninit chrdev_txs */

    for (struct chrdev_tx* chrdev_tx = self->chrdev_tx;
                           chrdev_tx < self->chrdev_tx + self->chrdev_tx_max_index;
                         ++chrdev_tx) {
        chrdev_tx_uninit(chrdev_tx);
    }

    /* Uninit regfile_txs */

    for (struct regfile_tx* regfile_tx = self->regfile_tx;
                            regfile_tx < self->regfile_tx + self->regfile_tx_max_index;
                          ++regfile_tx) {
        regfile_tx_uninit(regfile_tx);
    }

    /* Uninit socket_txs */

    for (struct socket_tx* socket_tx = self->socket_tx;
                           socket_tx < self->socket_tx + self->socket_tx_max_index;
                         ++socket_tx) {
        socket_tx_uninit(socket_tx);
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

static struct fifo_tx*
get_fifo_tx(struct fildes_tx* self, int index)
{
    for (struct fifo_tx* fifo_tx = self->fifo_tx + self->fifo_tx_max_index;
                         fifo_tx < self->fifo_tx + index + 1;
                       ++fifo_tx) {
        fifo_tx_init(fifo_tx);
    }

    self->fifo_tx_max_index = lmax(index + 1, self->fifo_tx_max_index);

    return self->fifo_tx + index;
}

static struct fifo_tx*
get_fifo_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                     struct picotm_error* error)
{
    struct ofd* ofd = ofdtab_ref_fildes(fildes,
                                        !!(flags & OFD_FL_WANTNEW),
                                        !!(flags & OFD_FL_UNLINK),
                                        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fifo_tx* fifo_tx = get_fifo_tx(self, ofdtab_index(ofd));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each FIFO. This reference is released
     * in fildes_tx_finish().
     */
    if (fifo_tx_holds_ref(fifo_tx)) {
        goto unref;
    }

    fifo_tx_ref_or_set_up(fifo_tx, ofd, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_fifo_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->fifo_tx_active_list, fifo_tx, active_list);

unref:
    ofd_unref(ofd);

    return fifo_tx;

err_fifo_tx_ref_or_set_up:
    ofd_unref(ofd);
    return NULL;
}

static struct chrdev_tx*
get_chrdev_tx(struct fildes_tx* self, int index)
{
    for (struct chrdev_tx* chrdev_tx = self->chrdev_tx + self->chrdev_tx_max_index;
                           chrdev_tx < self->chrdev_tx + index + 1;
                         ++chrdev_tx) {
        chrdev_tx_init(chrdev_tx);
    }

    self->chrdev_tx_max_index = lmax(index + 1, self->chrdev_tx_max_index);

    return self->chrdev_tx + index;
}

static struct chrdev_tx*
get_chrdev_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                       struct picotm_error* error)
{
    struct ofd* ofd = ofdtab_ref_fildes(fildes,
                                        !!(flags & OFD_FL_WANTNEW),
                                        !!(flags & OFD_FL_UNLINK),
                                        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct chrdev_tx* chrdev_tx = get_chrdev_tx(self, ofdtab_index(ofd));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each character device. This reference is
     * released in fildes_tx_finish().
     */
    if (chrdev_tx_holds_ref(chrdev_tx)) {
        goto unref;
    }

    chrdev_tx_ref_or_set_up(chrdev_tx, ofd, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_chrdev_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->chrdev_tx_active_list, chrdev_tx, active_list);

unref:
    ofd_unref(ofd);

    return chrdev_tx;

err_chrdev_tx_ref_or_set_up:
    ofd_unref(ofd);
    return NULL;
}

static struct regfile_tx*
get_regfile_tx(struct fildes_tx* self, int index)
{
    for (struct regfile_tx* regfile_tx = self->regfile_tx + self->regfile_tx_max_index;
                            regfile_tx < self->regfile_tx + index + 1;
                          ++regfile_tx) {
        regfile_tx_init(regfile_tx);
    }

    self->regfile_tx_max_index = lmax(index + 1, self->regfile_tx_max_index);

    return self->regfile_tx + index;
}

static struct regfile_tx*
get_regfile_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                        struct picotm_error* error)
{
    struct ofd* ofd = ofdtab_ref_fildes(fildes,
                                        !!(flags & OFD_FL_WANTNEW),
                                        !!(flags & OFD_FL_UNLINK),
                                        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct regfile_tx* regfile_tx = get_regfile_tx(self, ofdtab_index(ofd));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each regular file. This reference is
     * released in fildes_tx_finish().
     */
    if (regfile_tx_holds_ref(regfile_tx)) {
        goto unref;
    }

    regfile_tx_ref_or_set_up(regfile_tx, ofd, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_regfile_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->regfile_tx_active_list, regfile_tx, active_list);

unref:
    ofd_unref(ofd);

    return regfile_tx;

err_regfile_tx_ref_or_set_up:
    ofd_unref(ofd);
    return NULL;
}

static struct socket_tx*
get_socket_tx(struct fildes_tx* self, int index)
{
    for (struct socket_tx* socket_tx = self->socket_tx + self->socket_tx_max_index;
                           socket_tx < self->socket_tx + index + 1;
                         ++socket_tx) {
        socket_tx_init(socket_tx);
    }

    self->socket_tx_max_index = lmax(index + 1, self->socket_tx_max_index);

    return self->socket_tx + index;
}

static struct socket_tx*
get_socket_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                       struct picotm_error* error)
{
    struct ofd* ofd = ofdtab_ref_fildes(fildes,
                                        !!(flags & OFD_FL_WANTNEW),
                                        !!(flags & OFD_FL_UNLINK),
                                        error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct socket_tx* socket_tx = get_socket_tx(self, ofdtab_index(ofd));

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each socket. This reference is released
     * in fildes_tx_finish().
     */
    if (socket_tx_holds_ref(socket_tx)) {
        goto unref;
    }

    socket_tx_ref_or_set_up(socket_tx, ofd, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_socket_tx_ref_or_set_up;
    }

    SLIST_INSERT_HEAD(&self->socket_tx_active_list, socket_tx, active_list);

unref:
    ofd_unref(ofd);

    return socket_tx;

err_socket_tx_ref_or_set_up:
    ofd_unref(ofd);
    return NULL;
}

static struct ofd_tx*
get_ofd_tx_with_ref(struct fildes_tx* self, int fildes, unsigned long flags,
                    struct picotm_error* error)
{
    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    switch (buf.st_mode & S_IFMT) {
        case S_IFIFO: {
            struct fifo_tx* fifo_tx = get_fifo_tx_with_ref(self,
                                                           fildes,
                                                           flags,
                                                           error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &fifo_tx->base;
        }
        case S_IFCHR: {
            struct chrdev_tx* chrdev_tx = get_chrdev_tx_with_ref(self,
                                                                 fildes,
                                                                 flags,
                                                                 error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &chrdev_tx->base;
        }
        case S_IFREG: {
            struct regfile_tx* regfile_tx = get_regfile_tx_with_ref(self,
                                                                    fildes,
                                                                    flags,
                                                                    error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &regfile_tx->base;
        }
        case S_IFSOCK: {
            struct socket_tx* socket_tx = get_socket_tx_with_ref(self,
                                                                 fildes,
                                                                 flags,
                                                                 error);
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return &socket_tx->base;
        }
        default:
            fprintf(stderr, "file type = %x\n", buf.st_mode & S_IFMT);
            picotm_error_set_errno(error, EINVAL); /* unsupported file type */
            return NULL;
    }
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

    /* In |struct fildes_tx| we hold at most one reference to the
     * transaction state of each file descriptor. This reference
     * is released in fildes_tx_finish().
     */
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

    struct fd* fd = fdtab_ref_fildes(fildes, !!(flags & FD_FL_WANTNEW), error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct ofd_tx* ofd_tx = get_ofd_tx_with_ref(self, fildes, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_get_ofd_tx_with_ref;
    }

    fd_tx_ref_or_set_up(fd_tx, fd, ofd_tx, flags, error);
    if (picotm_error_is_set(error)) {
        goto err_fd_tx_ref;
    }

    SLIST_INSERT_HEAD(&self->fd_tx_active_list, fd_tx, active_list);

    fd_unref(fd);

    return fd_tx;

err_fd_tx_ref:
    ofd_tx_unref(ofd_tx);
err_get_ofd_tx_with_ref:
    fd_unref(fdtab + fildes);
    return NULL;
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

    get_fd_tx_with_ref(self, sockfd, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Accept connection */

    int connfd = TEMP_FAILURE_RETRY(accept(sockfd, address, address_len));
    if (connfd < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    /* Reference fd_tx */

    get_fd_tx_with_ref(self, connfd, 0, error);
    if (picotm_error_is_set(error)) {
        goto err_get_fd_tx_with_ref;
    }

    /* Inject event */
    append_cmd(self, CMD_ACCEPT, connfd, -1, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    return connfd;

err_get_fd_tx_with_ref:
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

    /* Bind */

    int cookie = -1;

    int res = fd_tx_bind_exec(fd_tx, socket, address, addresslen, &cookie,
                              isnoundo, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_bind_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, ev->fildes);
    assert(fd_tx);

    fd_tx_bind_undo(fd_tx, ev->fildes, ev->cookie, error);
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

    /* Connect */

    int cookie = -1;

    int res = fd_tx_connect_exec(fd_tx, sockfd, serv_addr, addrlen, &cookie,
                                 isnoundo, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_connect_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_connect_undo(fd_tx, fildes, cookie, error);
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

    /* Reference fd_tx for fildes2 */

    get_fd_tx_with_ref(self, fildes2, 0, error);
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

    int res = fd_tx_fcntl_exec(fd_tx, fildes, cmd, arg, &cookie, isnoundo,
                               error);
    if (picotm_error_is_set(error)) {
        return -1;
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

    fd_tx_fcntl_apply(fd_tx, event->fildes, event->cookie, event, error);
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

    fd_tx_fcntl_undo(fd_tx, fildes, cookie, error);
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

    /* Fsync */

    int cookie = -1;

    int res = fd_tx_fsync_exec(fd_tx, fildes, isnoundo, &cookie,
                               error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_fsync_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_fsync_undo(fd_tx, fildes, cookie, error);
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

    /* Connect */

    int cookie = -1;

    int res = fd_tx_listen_exec(fd_tx, sockfd, backlog, &cookie, isnoundo,
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_listen_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_listen_undo(fd_tx, fildes, cookie, error);
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

    /* Seek */

    int cookie = -1;

    off_t pos = fd_tx_lseek_exec(fd_tx, fildes, offset, whence, &cookie,
                                 isnoundo, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_lseek_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_lseek_undo(fd_tx, fildes, cookie, error);
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

    get_fd_tx_with_ref(self, fildes, FD_FL_WANTNEW, error);
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

    get_fd_tx_with_ref(self, pipefd[0], 0, error);
    if (picotm_error_is_set(error)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return -1;
    }

    get_fd_tx_with_ref(self, pipefd[1], 0, error);
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

    /* pread */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = fd_tx_pread_exec(fd_tx, fildes, buf, nbyte, off, &cookie,
                                   isnoundo, val_mode, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_pread_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_pread_undo(fd_tx, fildes, cookie, error);
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

    /* Pwrite */

    int cookie = -1;

    ssize_t len = fd_tx_pwrite_exec(fd_tx, fildes, buf, nbyte, off, &cookie,
                                    isnoundo, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_pwrite_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_pwrite_undo(fd_tx, fildes, cookie, error);
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

    /* Read */

    enum picotm_libc_validation_mode val_mode =
        fildes_tx_get_validation_mode(self);

    int cookie = -1;

    ssize_t len = fd_tx_read_exec(fd_tx, fildes, buf, nbyte, &cookie,
                                  isnoundo, val_mode, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_read_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_read_undo(fd_tx, fildes, cookie, error);
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

    /* Receive */

    int cookie = -1;

    ssize_t len = fd_tx_recv_exec(fd_tx, sockfd, buffer, length, flags,
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_recv_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_recv_undo(fd_tx, fildes, cookie, error);
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

    /* Send */

    int cookie = -1;

    ssize_t len = fd_tx_send_exec(fd_tx, sockfd, buffer, length, flags,
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_send_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_send_undo(fd_tx, fildes, cookie, error);
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

    /* Shutdown */

    int cookie = -1;

    int len = fd_tx_shutdown_exec(fd_tx, sockfd, how, &cookie, isnoundo,
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_shutdown_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_shutdown_undo(fd_tx, fildes, cookie, error);
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

    get_fd_tx_with_ref(self, sockfd, 0, error);
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

    /* Write */

    int cookie = -1;

    ssize_t len = fd_tx_write_exec(fd_tx, fildes, buf, nbyte, &cookie,
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

    struct fd_tx* fd_tx = get_fd_tx(self, event->fildes);
    assert(fd_tx);

    fd_tx_write_apply(fd_tx, event->fildes, event, error);
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

    struct fd_tx* fd_tx = get_fd_tx(self, fildes);
    assert(fd_tx);

    fd_tx_write_undo(fd_tx, fildes, cookie, error);
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
    /* Lock fds */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_lock(fd_tx);
    }

    /* Lock fifos */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_lock(fifo_tx);
    }

    /* Lock character devices */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_lock(chrdev_tx);
    }

    /* Lock regular files */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_lock(regfile_tx);
    }

    /* Lock sockets */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_lock(socket_tx);
    }
}

void
fildes_tx_unlock(struct fildes_tx* self)
{
    /* Unlock sockets */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_unlock(socket_tx);
    }

    /* Unlock regular files */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_unlock(regfile_tx);
    }

    /* Unlock character devices */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_unlock(chrdev_tx);
    }

    /* Unlock fifos */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_unlock(fifo_tx);
    }

    /* Unlock fds */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_unlock(fd_tx);
    }
}

void
fildes_tx_validate(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Validate fd_txs */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_validate(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate fifo_txs */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_validate(fifo_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate chrdev_txs */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_validate(chrdev_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate regfile_txs */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_validate(regfile_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Validate socket_txs */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_validate(socket_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
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

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_update_cc(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update fifo_txs */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_update_cc(fifo_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update chrdev_txs */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_update_cc(chrdev_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update regfile_txs */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_update_cc(regfile_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Update socket_txs */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_update_cc(socket_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

void
fildes_tx_clear_cc(struct fildes_tx* self, int noundo,
                   struct picotm_error* error)
{
    /* Clear fd_txs' CC */

    struct fd_tx* fd_tx;
    SLIST_FOREACH(fd_tx, &self->fd_tx_active_list, active_list) {
        fd_tx_clear_cc(fd_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear fifo_txs' CC */

    struct fifo_tx* fifo_tx;
    SLIST_FOREACH(fifo_tx, &self->fifo_tx_active_list, active_list) {
        fifo_tx_clear_cc(fifo_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear chrdev_txs' CC */

    struct chrdev_tx* chrdev_tx;
    SLIST_FOREACH(chrdev_tx, &self->chrdev_tx_active_list, active_list) {
        chrdev_tx_clear_cc(chrdev_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear regfile_txs' CC */

    struct regfile_tx* regfile_tx;
    SLIST_FOREACH(regfile_tx, &self->regfile_tx_active_list, active_list) {
        regfile_tx_clear_cc(regfile_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    /* Clear socket_txs' CC */

    struct socket_tx* socket_tx;
    SLIST_FOREACH(socket_tx, &self->socket_tx_active_list, active_list) {
        socket_tx_clear_cc(socket_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

void
fildes_tx_finish(struct fildes_tx* self)
{
    /* Unref socket_txs */

    while (!SLIST_EMPTY(&self->socket_tx_active_list)) {

        struct socket_tx* socket_tx = SLIST_FIRST(&self->socket_tx_active_list);
        SLIST_REMOVE_HEAD(&self->socket_tx_active_list, active_list);

        socket_tx_unref(socket_tx);
    }

    /* Unref regfile_txs */

    while (!SLIST_EMPTY(&self->regfile_tx_active_list)) {

        struct regfile_tx* regfile_tx = SLIST_FIRST(&self->regfile_tx_active_list);
        SLIST_REMOVE_HEAD(&self->regfile_tx_active_list, active_list);

        regfile_tx_unref(regfile_tx);
    }

    /* Unref chrdev_txs */

    while (!SLIST_EMPTY(&self->chrdev_tx_active_list)) {

        struct chrdev_tx* chrdev_tx = SLIST_FIRST(&self->chrdev_tx_active_list);
        SLIST_REMOVE_HEAD(&self->chrdev_tx_active_list, active_list);

        chrdev_tx_unref(chrdev_tx);
    }

    /* Unref fifo_txs */

    while (!SLIST_EMPTY(&self->fifo_tx_active_list)) {

        struct fifo_tx* fifo_tx = SLIST_FIRST(&self->fifo_tx_active_list);
        SLIST_REMOVE_HEAD(&self->fifo_tx_active_list, active_list);

        fifo_tx_unref(fifo_tx);
    }

    /* Unref fd_txs */

    while (!SLIST_EMPTY(&self->fd_tx_active_list)) {

        struct fd_tx* fd_tx = SLIST_FIRST(&self->fd_tx_active_list);
        SLIST_REMOVE_HEAD(&self->fd_tx_active_list, active_list);

        fd_tx_unref(fd_tx);
    }
}
