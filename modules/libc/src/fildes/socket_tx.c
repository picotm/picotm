/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "socket_tx.h"

#include "fcntlop.h"
#include "fcntloptab.h"
#include "file_tx_ops.h"
#include "ioop.h"
#include "iooptab.h"
#include "sockbuf_tx.h"
#include "socket_tx.h"

#include "compat/temp_failure_retry.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-tab.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static void
init_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_init(beg);
        ++beg;
    }
}

static void
uninit_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_uninit(beg);
        ++beg;
    }
}

/*
 * struct file_tx_ops
 */

static void
socket_tx_op_prepare(struct file_tx* file_tx, struct file* file, void* data,
                     struct picotm_error* error)
{
    socket_tx_prepare(socket_tx_of_file_tx(file_tx), socket_of_base(file),
                      error);
}

static void
socket_tx_op_release(struct file_tx* file_tx)
{
    socket_tx_release(socket_tx_of_file_tx(file_tx));
}

static void
socket_tx_op_finish(struct file_tx* base)
{
    socket_tx_finish(socket_tx_of_file_tx(base));
}

/* accept() */

static int
socket_tx_op_accept_exec(struct file_tx* base,
                         int sockfd, struct sockaddr* address, socklen_t* address_len,
                         bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    int res = TEMP_FAILURE_RETRY(accept(sockfd, address, address_len));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

/* bind() */

static int
socket_tx_op_bind_exec(struct file_tx* base,
                       int sockfd, const struct sockaddr* address, socklen_t address_len,
                       bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    int res = bind(sockfd, address, address_len);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

/* connect() */

static int
socket_tx_op_connect_exec(struct file_tx* base,
                          int sockfd, const struct sockaddr* address, socklen_t address_len,
                          bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    int res = TEMP_FAILURE_RETRY(connect(sockfd, address, address_len));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

/* fcntl() */

static int
socket_tx_op_fcntl_exec(struct file_tx* base,
                        int fildes, int cmd, union fcntl_arg* arg,
                        bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);

    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
        case F_GETOWN: {

            /* Read-lock socket */
            socket_tx_try_rdlock_field(self, SOCKET_FIELD_STATE, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd));
            arg->arg0 = res;
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        case F_GETLK: {

            /* Read-lock socket */
            socket_tx_try_rdlock_field(self, SOCKET_FIELD_STATE, error);
            if (picotm_error_is_set(error)) {
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        case F_SETFL:
        case F_SETFD:
        case F_SETOWN: {

            if (!isnoundo) {
                picotm_error_set_revocable(error);
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg0));
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        case F_SETLK:
        case F_SETLKW: {

            if (!isnoundo) {
                picotm_error_set_revocable(error);
                return -1;
            }

            int res = TEMP_FAILURE_RETRY(fcntl(fildes, cmd, arg->arg1));
            if (res < 0) {
                picotm_error_set_errno(error, errno);
                return res;
            }
            return res;
        }
        default:
            picotm_error_set_errno(error, EINVAL);
            return -1;
    }
}

/* fstat() */

static int
socket_tx_op_fstat_exec(struct file_tx* base,
                        int fildes, struct stat* buf,
                        bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);

    /* Acquire file-mode reader lock. */
    socket_tx_try_rdlock_field(self, SOCKET_FIELD_FILE_MODE, error);
    if (picotm_error_is_set(error)) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    int res = fstat(fildes, buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

/* listen() */

static int
socket_tx_op_listen_exec(struct file_tx* base,
                         int sockfd, int backlog,
                         bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    int res = listen(sockfd, backlog);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

/* read() */

static bool
errno_signals_blocking_io(int errno_code)
{
    return (errno_code == EAGAIN) || (errno_code == EWOULDBLOCK);
}

static ssize_t
do_read(int fildes, void* buf, size_t nbyte, struct picotm_error* error)
{
    uint8_t* pos = buf;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + nbyte;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(read(fildes, pos, end - pos));
        if (res < 0) {
            if (pos != beg) {
                break; /* return read data */
            } else if (errno_signals_blocking_io(errno)) {
                return -1; /* error for non-blocking I/O */
            } else {
                picotm_error_set_errno(error, errno);
                return res;
            }
        } else if (!res) {
            break; /* EOF reached */
        }

        pos += res;
    }

    return pos - beg;
}

static ssize_t
socket_tx_op_read_exec(struct file_tx* base,
                       int fildes, void* buf, size_t nbyte,
                       bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    struct socket_tx* self = socket_tx_of_file_tx(base);
    struct sockbuf_tx* sockbuf_tx = self->sockbuf_tx;

    sockbuf_tx_try_wrlock_field(sockbuf_tx, SOCKBUF_FIELD_RECV_END, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    ssize_t res = do_read(fildes, buf, nbyte, error);
    if (picotm_error_is_set(error)) {
        return res;
    }
    return res;
}

/* recv() */

static ssize_t
do_recv(int sockfd, void* buffer, size_t length, int flags,
        struct picotm_error* error)
{
    uint8_t* pos = buffer;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + length;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(recv(sockfd, pos, end - pos, flags));
        if (res < 0) {
            if (pos != beg) {
                break; /* return read data */
            } else if (errno_signals_blocking_io(errno)) {
                return -1; /* error for non-blocking I/O */
            } else {
                picotm_error_set_errno(error, errno);
                return res;
            }
        } else if (!res) {
            break; /* EOF reached */
        }

        pos += res;
    }

    return pos - beg;
}

static ssize_t
socket_tx_op_recv_exec(struct file_tx* base,
                       int sockfd, void* buffer, size_t length, int flags,
                       bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    struct socket_tx* self = socket_tx_of_file_tx(base);
    struct sockbuf_tx* sockbuf_tx = self->sockbuf_tx;

    sockbuf_tx_try_wrlock_field(sockbuf_tx, SOCKBUF_FIELD_RECV_END, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    ssize_t res = do_recv(sockfd, buffer, length, flags, error);
    if (picotm_error_is_set(error)) {
        return res;
    }
    return res;
}

/* send() */

static ssize_t
do_send(int sockfd, const void* buffer, size_t length, int flags,
        struct picotm_error* error)
{
    const uint8_t* pos = buffer;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + length;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(send(sockfd, pos, end - pos, flags));
        if (res < 0) {
            if (pos != beg) {
                break; /* return sent data */
            } else if (errno_signals_blocking_io(errno)) {
                return -1; /* error for non-blocking I/O */
            } else {
                picotm_error_set_errno(error, errno);
                return res;
            }
        }

        pos += res;
    }

    return pos - beg;
}

static ssize_t
socket_tx_op_send_exec(struct file_tx* base,
                       int sockfd, const void* buffer, size_t length, int flags,
                       bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);
    struct sockbuf_tx* sockbuf_tx = self->sockbuf_tx;

    if (isnoundo) {
        sockbuf_tx->wrmode = PICOTM_LIBC_WRITE_THROUGH;
    } else {
        if (sockbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
            picotm_error_set_revocable(error);
            return -1;
        }
        /* ATM no send flags are supported for speculative execution. In
         * the future, a more fine-grained selection of send flags could
         * be made.
         */
        if (flags) {
            picotm_error_set_revocable(error);
            return -1;
        }
    }

    /* Write-lock socket write end */
    sockbuf_tx_try_wrlock_field(sockbuf_tx, SOCKBUF_FIELD_SEND_END, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    ssize_t res;

    if (sockbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        res = do_send(sockfd, buffer, length, flags, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
    } else {

        /* Register write data */
        *cookie = sockbuf_tx_append_to_writeset(sockbuf_tx, length, 0, buffer,
                                                error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        res = length;
    }

    return res;
}

static void
socket_tx_op_send_apply(struct file_tx* base, int sockfd, int cookie,
                        struct picotm_error* error)
{
    static const int flags = 0;

    struct socket_tx* self = socket_tx_of_file_tx(base);
    struct sockbuf_tx* sockbuf_tx = self->sockbuf_tx;

    if (sockbuf_tx->wrmode == PICOTM_LIBC_WRITE_BACK) {
        do_send(sockfd,
                sockbuf_tx->wrbuf + sockbuf_tx->wrtab[cookie].bufoff,
                sockbuf_tx->wrtab[cookie].nbyte, flags, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

/* shutdown() */

static int
socket_tx_op_shutdown_exec(struct file_tx* base,
                           int sockfd, int how,
                           bool isnoundo, int* cookie, struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    int res = shutdown(sockfd, how);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

/* write() */
static ssize_t
do_write(int fildes, const void* buf, size_t nbyte,
         struct picotm_error* error)
{
    const uint8_t* pos = buf;
    const uint8_t* beg = pos;
    const uint8_t* end = beg + nbyte;

    while (pos < end) {

        ssize_t res = TEMP_FAILURE_RETRY(write(fildes, pos, end - pos));
        if (res < 0) {
            if (pos != beg) {
                break; /* return written data */
            } else if (errno_signals_blocking_io(errno)) {
                return -1; /* error for non-blocking I/O */
            } else {
                picotm_error_set_errno(error, errno);
                return res;
            }
        }

        pos += res;
    }

    return pos - beg;
}

static ssize_t
socket_tx_op_write_exec(struct file_tx* base,
                        int fildes, const void* buf, size_t nbyte,
                        bool isnoundo, int* cookie, struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);
    struct sockbuf_tx* sockbuf_tx = self->sockbuf_tx;

    if (isnoundo) {
        sockbuf_tx->wrmode = PICOTM_LIBC_WRITE_THROUGH;
    } else if (sockbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* Write-lock socket write end */
    sockbuf_tx_try_wrlock_field(sockbuf_tx, SOCKBUF_FIELD_SEND_END, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    ssize_t res;

    if (sockbuf_tx->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        res = do_write(fildes, buf, nbyte, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
    } else {

        /* Register write data */
        *cookie = sockbuf_tx_append_to_writeset(sockbuf_tx, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        res = nbyte;
    }

    return res;
}

static void
socket_tx_op_write_apply(struct file_tx* base, int fildes, int cookie,
                         struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);
    struct sockbuf_tx* sockbuf_tx = self->sockbuf_tx;

    if (sockbuf_tx->wrmode == PICOTM_LIBC_WRITE_BACK) {
        do_write(fildes,
                 sockbuf_tx->wrbuf + sockbuf_tx->wrtab[cookie].bufoff,
                 sockbuf_tx->wrtab[cookie].nbyte, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

static const struct file_tx_ops socket_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_SOCKET,
    /* file handling */
    .prepare = socket_tx_op_prepare,
    .release = socket_tx_op_release,
    /* module interfaces */
    .finish = socket_tx_op_finish,
    /* file ops */
    .accept_exec = socket_tx_op_accept_exec,
    .accept_apply = file_tx_op_apply,
    .accept_undo = file_tx_op_undo,
    .bind_exec = socket_tx_op_bind_exec,
    .bind_apply = file_tx_op_apply,
    .bind_undo = file_tx_op_undo,
    .connect_exec = socket_tx_op_connect_exec,
    .connect_apply = file_tx_op_apply,
    .connect_undo = file_tx_op_undo,
    .fchmod_exec = file_tx_op_fchmod_exec_einval,
    .fcntl_exec = socket_tx_op_fcntl_exec,
    .fcntl_apply = file_tx_op_apply,
    .fcntl_undo = file_tx_op_undo,
    .fstat_exec = socket_tx_op_fstat_exec,
    .fstat_apply = file_tx_op_apply,
    .fstat_undo = file_tx_op_undo,
    .fsync_exec = file_tx_op_fsync_exec_einval,
    .listen_exec = socket_tx_op_listen_exec,
    .listen_apply = file_tx_op_apply,
    .listen_undo = file_tx_op_undo,
    .lseek_exec = file_tx_op_lseek_exec_espipe,
    .pread_exec = file_tx_op_pread_exec_espipe,
    .pwrite_exec = file_tx_op_pwrite_exec_espipe,
    .read_exec = socket_tx_op_read_exec,
    .read_apply = file_tx_op_apply,
    .read_undo = file_tx_op_undo,
    .recv_exec = socket_tx_op_recv_exec,
    .recv_apply = file_tx_op_apply,
    .recv_undo = file_tx_op_undo,
    .send_exec = socket_tx_op_send_exec,
    .send_apply = socket_tx_op_send_apply,
    .send_undo = file_tx_op_undo,
    .shutdown_exec = socket_tx_op_shutdown_exec,
    .shutdown_apply = file_tx_op_apply,
    .shutdown_undo = file_tx_op_undo,
    .write_exec = socket_tx_op_write_exec,
    .write_apply = socket_tx_op_write_apply,
    .write_undo = file_tx_op_undo,
};

/*
 * Public interface
 */

void
socket_tx_init(struct socket_tx* self)
{
    assert(self);

    file_tx_init(&self->base, &socket_tx_ops);

    self->sockbuf_tx = nullptr;
    self->fcntltab = nullptr;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
socket_tx_uninit(struct socket_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/*
 * File handling
 */

void
socket_tx_prepare(struct socket_tx* self, struct socket* socket,
                  struct picotm_error* error)
{
    assert(self);

    self->sockbuf_tx = nullptr;
    self->fcntltablen = 0;
}

void
socket_tx_release(struct socket_tx* self)
{ }

void
socket_tx_try_rdlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_rdlock_field(socket_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

void
socket_tx_try_wrlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_wrlock_field(socket_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct socket* socket)
{
    enum socket_field field = 0;

    while (beg < end) {
        socket_unlock_field(socket, field, beg);
        ++field;
        ++beg;
    }
}

void
socket_tx_finish(struct socket_tx* self)
{
    /* release reader/writer locks on socket state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    socket_of_base(self->base.file));
}
