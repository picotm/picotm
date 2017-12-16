/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#include "file_tx_ops.h"
#include <errno.h>
#include <picotm/picotm-error.h>

int
file_tx_op_accept_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                int sockfd, struct sockaddr* address,
                                socklen_t* address_len, bool isnoundo,
                                int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_bind_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int sockfd, const struct sockaddr* address,
                              socklen_t addresslen, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_connect_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                 int sockfd, const struct sockaddr* address,
                                 socklen_t addresslen, bool isnoundo,
                                 int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_fchmod_exec_einval(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int fildes, mode_t mode, bool isnoundo,
                              int* cookie, struct picotm_error* error)
{
    /* TODO: What happens when we call fchmod() on a socket? */

    picotm_error_set_errno(error, EINVAL);
    return -1;
}

int
file_tx_op_fsync_exec_einval(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return -1;
}

int
file_tx_op_listen_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                int sockfd, int backlog, bool isnoundo,
                                int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

off_t
file_tx_op_lseek_exec_einval(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, off_t offset, int whence,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EINVAL);
    return (off_t)-1;
}

off_t
file_tx_op_lseek_exec_espipe(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, off_t offset, int whence,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return (off_t)-1;
}

ssize_t
file_tx_op_pread_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, void* buf, size_t nbyte, off_t off,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

ssize_t
file_tx_op_pread_exec_espipe(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, void* buf, size_t nbyte, off_t off,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

ssize_t
file_tx_op_pwrite_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int fildes, const void* buf, size_t nbyte,
                              off_t off, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

ssize_t
file_tx_op_pwrite_exec_espipe(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int fildes, const void* buf, size_t nbyte,
                              off_t off, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ESPIPE);
    return -1;
}

ssize_t
file_tx_op_read_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                            int fildes, void* buf, size_t nbyte,
                            bool isnoundo, int* cookie,
                            struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

ssize_t
file_tx_op_recv_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int sockfd, void* buffer, size_t length,
                              int flags, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

ssize_t
file_tx_op_send_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int sockfd, const void* buffer, size_t length,
                              int flags, bool isnoundo, int* cookie,
                              struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

int
file_tx_op_shutdown_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                  int sockfd, int how, bool isnoundo,
                                  int* cookie, struct picotm_error* error)
{
    picotm_error_set_errno(error, ENOTSOCK);
    return -1;
}

ssize_t
file_tx_op_write_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, const void* buf, size_t nbyte,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error)
{
    picotm_error_set_errno(error, EISDIR);
    return -1;
}

void
file_tx_op_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
                 int cookie, struct picotm_error* error)
{ }

void
file_tx_op_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
                int cookie, struct picotm_error* error)
{ }
