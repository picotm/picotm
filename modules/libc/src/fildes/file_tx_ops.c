/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 * Copyright (c) 2020       Thomas Zimmermann <contact@tzimmermann.org>
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

#include "file_tx_ops.h"
#include "picotm/picotm-error.h"
#include <errno.h>

int
file_tx_op_accept_exec_enotsock(struct file_tx* base,
                                int sockfd, struct sockaddr* address,
                                socklen_t* address_len, bool isnoundo,
                                int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_bind_exec_enotsock(struct file_tx* base,
                              int sockfd, const struct sockaddr* address,
                              socklen_t addresslen, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_connect_exec_enotsock(struct file_tx* base,
                                 int sockfd, const struct sockaddr* address,
                                 socklen_t addresslen, bool isnoundo,
                                 int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_fchmod_exec_einval(struct file_tx* base,
                              int fildes, mode_t mode, bool isnoundo,
                              int* cookie, struct picotm_error* error)
{
    /* TODO: What happens when we call fchmod() on a socket? */

    picotm_error_set_errno(error, EINVAL);
    return -1;
}

int
file_tx_op_fsync_exec_einval(struct file_tx* base,
                             int fildes, bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return -1;
}

int
file_tx_op_listen_exec_enotsock(struct file_tx* base,
                                int sockfd, int backlog, bool isnoundo,
                                int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

off_t
file_tx_op_lseek_exec_einval(struct file_tx* base,
                             int fildes, off_t offset, int whence,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return (off_t)-1;
}

off_t
file_tx_op_lseek_exec_espipe(struct file_tx* base,
                             int fildes, off_t offset, int whence,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return (off_t)-1;
}

ssize_t
file_tx_op_pread_exec_eisdir(struct file_tx* base,
                             int fildes, void* buf, size_t nbyte, off_t off,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

ssize_t
file_tx_op_pread_exec_espipe(struct file_tx* base,
                             int fildes, void* buf, size_t nbyte, off_t off,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

ssize_t
file_tx_op_pwrite_exec_eisdir(struct file_tx* base,
                              int fildes, const void* buf, size_t nbyte,
                              off_t off, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

ssize_t
file_tx_op_pwrite_exec_espipe(struct file_tx* base,
                              int fildes, const void* buf, size_t nbyte,
                              off_t off, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

ssize_t
file_tx_op_read_exec_eisdir(struct file_tx* base,
                            int fildes, void* buf, size_t nbyte,
                            bool isnoundo, int* cookie,
                            struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

ssize_t
file_tx_op_recv_exec_enotsock(struct file_tx* base,
                              int sockfd, void* buffer, size_t length,
                              int flags, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

ssize_t
file_tx_op_send_exec_enotsock(struct file_tx* base,
                              int sockfd, const void* buffer, size_t length,
                              int flags, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_shutdown_exec_enotsock(struct file_tx* base,
                                  int sockfd, int how, bool isnoundo,
                                  int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

ssize_t
file_tx_op_write_exec_eisdir(struct file_tx* base,
                             int fildes, const void* buf, size_t nbyte,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

void
file_tx_op_apply(struct file_tx* base, int fildes, int cookie,
                 struct picotm_error* error)
{ }

void
file_tx_op_undo(struct file_tx* base, int sockfd, int cookie,
                struct picotm_error* error)
{ }
