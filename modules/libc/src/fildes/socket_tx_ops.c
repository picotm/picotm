/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "socket_tx_ops.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "compat/temp_failure_retry.h"
#include "fcntlop.h"
#include "fcntloptab.h"
#include "file_tx_ops.h"
#include "ioop.h"
#include "iooptab.h"
#include "socket_tx.h"

/*
 * Reference counting
 */

static void
ref(struct file_tx* file_tx, struct picotm_error* error)
{
    socket_tx_ref(socket_tx_of_file_tx(file_tx), error);
}

static void
unref(struct file_tx* file_tx)
{
    socket_tx_unref(socket_tx_of_file_tx(file_tx));
}

/*
 * Module interface
 */

static void
finish(struct file_tx* base)
{
    socket_tx_finish(socket_tx_of_file_tx(base));
}

/*
 * accept()
 */

static int
accept_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
            struct sockaddr* address, socklen_t* address_len, bool isnoundo,
            int* cookie, struct picotm_error* error)
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

/*
 * bind()
 */

static int
bind_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
          const struct sockaddr* address, socklen_t address_len,
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

/*
 * connect()
 */

static int
connect_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
             const struct sockaddr* address, socklen_t address_len,
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

/*
 * fcntl()
 */

static int
fcntl_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, int cmd,
           union fcntl_arg* arg, bool isnoundo, int* cookie,
           struct picotm_error* error)
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

/*
 * fstat()
 */

static int
fstat_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           struct stat* buf, bool isnoundo, int* cookie,
           struct picotm_error* error)
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

/*
 * listen()
 */

static int
listen_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
            int backlog, bool isnoundo, int* cookie,
            struct picotm_error* error)
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

/*
 * read()
 */

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
read_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes, void* buf,
          size_t nbyte, bool isnoundo, int* cookie,
          struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    struct socket_tx* self = socket_tx_of_file_tx(base);

    socket_tx_try_wrlock_field(self, SOCKET_FIELD_RECV_END, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    ssize_t res = do_read(fildes, buf, nbyte, error);
    if (picotm_error_is_set(error)) {
        return res;
    }
    return res;
}

/*
 * recv()
 */

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
recv_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
          void* buffer, size_t length, int flags, bool isnoundo, int* cookie,
          struct picotm_error* error)
{
    if (!isnoundo) {
        picotm_error_set_revocable(error);
        return -1;
    }

    struct socket_tx* self = socket_tx_of_file_tx(base);

    socket_tx_try_wrlock_field(self, SOCKET_FIELD_RECV_END, error);
    if (picotm_error_is_set(error)) {
        return (off_t)-1;
    }

    ssize_t res = do_recv(sockfd, buffer, length, flags, error);
    if (picotm_error_is_set(error)) {
        return res;
    }
    return res;
}

/*
 * send()
 */

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
send_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
          const void* buffer, size_t length, int flags, bool isnoundo,
          int* cookie, struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);

    if (isnoundo) {
        self->wrmode = PICOTM_LIBC_WRITE_THROUGH;
    } else {
        if (self->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
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
    socket_tx_try_wrlock_field(self, SOCKET_FIELD_SEND_END, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    ssize_t res;

    if (self->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        res = do_send(sockfd, buffer, length, flags, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
    } else {

        /* Register write data */
        *cookie = socket_tx_append_to_writeset(self, length, 0, buffer, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        res = length;
    }

    return res;
}

static void
send_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
           int cookie, struct picotm_error* error)
{
    static const int flags = 0;

    struct socket_tx* self = socket_tx_of_file_tx(base);

    if (self->wrmode == PICOTM_LIBC_WRITE_BACK) {
        do_send(sockfd,
                self->wrbuf + self->wrtab[cookie].bufoff,
                self->wrtab[cookie].nbyte, flags, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

/*
 * shutdown()
 */

static int
shutdown_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
              int how, bool isnoundo, int* cookie, struct picotm_error* error)
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

/*
 * write()
 */

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
write_exec(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
           const void* buf, size_t nbyte, bool isnoundo, int* cookie,
           struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);

    if (isnoundo) {
        self->wrmode = PICOTM_LIBC_WRITE_THROUGH;
    } else if (self->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        picotm_error_set_revocable(error);
        return -1;
    }

    /* Write-lock socket write end */
    socket_tx_try_wrlock_field(self, SOCKET_FIELD_SEND_END, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    ssize_t res;

    if (self->wrmode == PICOTM_LIBC_WRITE_THROUGH) {
        res = do_write(fildes, buf, nbyte, error);
        if (picotm_error_is_set(error)) {
            return res;
        }
    } else {

        /* Register write data */
        *cookie = socket_tx_append_to_writeset(self, nbyte, 0, buf, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        res = nbyte;
    }

    return res;
}

static void
write_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int fildes,
            int cookie, struct picotm_error* error)
{
    struct socket_tx* self = socket_tx_of_file_tx(base);

    if (self->wrmode == PICOTM_LIBC_WRITE_BACK) {
        do_write(fildes,
                 self->wrbuf + self->wrtab[cookie].bufoff,
                 self->wrtab[cookie].nbyte, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

/*
 * Public interface
 */

const struct file_tx_ops socket_tx_ops = {
    PICOTM_LIBC_FILE_TYPE_SOCKET,
    /* ref counting */
    ref,
    unref,
    /* module interfaces */
    finish,
    /* file ops */
    accept_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    bind_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    connect_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    file_tx_op_fchmod_exec_einval,
    NULL,
    NULL,
    fcntl_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    fstat_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    file_tx_op_fsync_exec_einval,
    NULL,
    NULL,
    listen_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    file_tx_op_lseek_exec_espipe,
    NULL,
    NULL,
    file_tx_op_pread_exec_espipe,
    NULL,
    NULL,
    file_tx_op_pwrite_exec_espipe,
    NULL,
    NULL,
    read_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    recv_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    send_exec,
    send_apply,
    file_tx_op_undo,
    shutdown_exec,
    file_tx_op_apply,
    file_tx_op_undo,
    write_exec,
    write_apply,
    file_tx_op_undo
};
