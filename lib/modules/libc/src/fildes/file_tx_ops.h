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

#pragma once

#include <stdbool.h>
#include <sys/socket.h>
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct file_tx;
struct ofd_tx;
struct picotm_error;
struct stat;

union fcntl_arg;

/**
 * \brief File operations.
 */
struct file_tx_ops {

    enum picotm_libc_file_type type;

    /*
     * Reference counting
     */

    void (*ref)(struct file_tx*, struct picotm_error*);
    void (*unref)(struct file_tx*);

    /*
     * Module interfaces
     */

    void (*lock)(struct file_tx*, struct picotm_error*);
    void (*unlock)(struct file_tx*, struct picotm_error*);
    void (*validate)(struct file_tx*, struct picotm_error*r);
    void (*update_cc)(struct file_tx*, struct picotm_error*);
    void (*clear_cc)(struct file_tx*, struct picotm_error*);

    /*
     * accept()
     */

    int (*accept_exec)(struct file_tx*, struct ofd_tx*, int,
                       struct sockaddr*, socklen_t*, bool, int*,
                       struct picotm_error*);

    void (*accept_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*accept_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * bind()
     */

    int (*bind_exec)(struct file_tx*, struct ofd_tx*, int,
                     const struct sockaddr*, socklen_t, bool, int*,
                     struct picotm_error*);

    void (*bind_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*bind_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * connect()
     */

    int (*connect_exec)(struct file_tx*, struct ofd_tx*, int,
                        const struct sockaddr*, socklen_t, bool, int*,
                        struct picotm_error*);

    void (*connect_apply)(struct file_tx*, struct ofd_tx*, int, int,
                          struct picotm_error*);

    void (*connect_undo)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    /*
     * fchmod()
     */

    int (*fchmod_exec)(struct file_tx*, struct ofd_tx*, int, mode_t, bool,
                       int*, struct picotm_error*);

    void (*fchmod_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*fchmod_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * fcntl()
     */

    int (*fcntl_exec)(struct file_tx*, struct ofd_tx*, int, int,
                      union fcntl_arg*, bool, int*, struct picotm_error*);

    void (*fcntl_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*fcntl_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * fstat()
     */

    int (*fstat_exec)(struct file_tx*, struct ofd_tx*, int, struct stat*,
                      bool, int*, struct picotm_error*);

    void (*fstat_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*fstat_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * fsync()
     */

    int (*fsync_exec)(struct file_tx*, struct ofd_tx*, int, bool, int*,
                      struct picotm_error*);

    void (*fsync_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*fsync_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * listen()
     */

    int (*listen_exec)(struct file_tx*, struct ofd_tx*, int, int, bool, int*,
                       struct picotm_error*);

    void (*listen_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*listen_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * lseek()
     */

    off_t (*lseek_exec)(struct file_tx*, struct ofd_tx*, int, off_t, int,
                        bool, int*, struct picotm_error*);

    void (*lseek_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*lseek_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * pread()
     */

    ssize_t (*pread_exec)(struct file_tx*, struct ofd_tx*, int, void*, size_t,
                          off_t, bool, int*, struct picotm_error*);

    void (*pread_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*pread_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    /*
     * pwrite()
     */

    ssize_t (*pwrite_exec)(struct file_tx*, struct ofd_tx*, int, const void*,
                           size_t, off_t, bool, int*, struct picotm_error*);

    void (*pwrite_apply)(struct file_tx*, struct ofd_tx*, int, int,
                         struct picotm_error*);

    void (*pwrite_undo)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    /*
     * read()
     */

    ssize_t (*read_exec)(struct file_tx*, struct ofd_tx*, int, void *buf,
                         size_t, bool, int*, struct picotm_error*);

    void (*read_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*read_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * recv()
     */

    ssize_t (*recv_exec)(struct file_tx*, struct ofd_tx*, int, void*, size_t,
                         int, bool, int*, struct picotm_error*);

    void (*recv_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*recv_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * send()
     */

    ssize_t (*send_exec)(struct file_tx*, struct ofd_tx*, int, const void*,
                         size_t, int, bool, int*, struct picotm_error*);

    void (*send_apply)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);

    void (*send_undo)(struct file_tx*, struct ofd_tx*, int, int,
                      struct picotm_error*);

    /*
     * shutdown()
     */

    int (*shutdown_exec)(struct file_tx*, struct ofd_tx*, int, int, bool,
                         int*, struct picotm_error*);

    void (*shutdown_apply)(struct file_tx*, struct ofd_tx*, int, int,
                           struct picotm_error*);

    void (*shutdown_undo)(struct file_tx*, struct ofd_tx*, int, int,
                          struct picotm_error*);

    /*
     * write()
     */

    ssize_t (*write_exec)(struct file_tx*, struct ofd_tx*, int, const void*,
                          size_t, bool, int*, struct picotm_error*);

    void (*write_apply)(struct file_tx*, struct ofd_tx*, int, int,
                        struct picotm_error*);

    void (*write_undo)(struct file_tx*, struct ofd_tx*, int, int,
                       struct picotm_error*);
};

int
file_tx_op_accept_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                int sockfd, struct sockaddr* address,
                                socklen_t* address_len, bool isnoundo,
                                int* cookie, struct picotm_error* error);

int
file_tx_op_bind_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int sockfd, const struct sockaddr* address,
                              socklen_t addresslen, bool isnoundo, int* cookie,
                              struct picotm_error* error);

int
file_tx_op_connect_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                 int sockfd, const struct sockaddr* address,
                                 socklen_t addresslen, bool isnoundo,
                                 int* cookie, struct picotm_error* error);

int
file_tx_op_fchmod_exec_einval(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int fildes, mode_t mode, bool isnoundo,
                              int* cookie, struct picotm_error* error);

int
file_tx_op_fsync_exec_einval(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, bool isnoundo, int* cookie,
                             struct picotm_error* error);

int
file_tx_op_listen_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                int sockfd, int backlog, bool isnoundo,
                                int* cookie, struct picotm_error* error);

off_t
file_tx_op_lseek_exec_einval(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, off_t offset, int whence,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error);

off_t
file_tx_op_lseek_exec_espipe(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, off_t offset, int whence,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error);

ssize_t
file_tx_op_pread_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, void* buf, size_t nbyte, off_t off,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error);

ssize_t
file_tx_op_pread_exec_espipe(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, void* buf, size_t nbyte, off_t off,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error);

ssize_t
file_tx_op_pwrite_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int fildes, const void* buf, size_t nbyte,
                              off_t off, bool isnoundo, int* cookie,
                              struct picotm_error* error);

ssize_t
file_tx_op_pwrite_exec_espipe(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int fildes, const void* buf, size_t nbyte,
                              off_t off, bool isnoundo, int* cookie,
                              struct picotm_error* error);

ssize_t
file_tx_op_read_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                            int fildes, void* buf, size_t nbyte,
                            bool isnoundo, int* cookie,
                            struct picotm_error* error);

ssize_t
file_tx_op_recv_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int sockfd, void* buffer, size_t length,
                              int flags, bool isnoundo, int* cookie,
                              struct picotm_error* error);

ssize_t
file_tx_op_send_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                              int sockfd, const void* buffer, size_t length,
                              int flags, bool isnoundo, int* cookie,
                              struct picotm_error* error);

int
file_tx_op_shutdown_exec_enotsock(struct file_tx* base, struct ofd_tx* ofd_tx,
                                  int sockfd, int how, bool isnoundo,
                                  int* cookie, struct picotm_error* error);

ssize_t
file_tx_op_write_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, const void* buf, size_t nbyte,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error);

ssize_t
file_tx_opswrite_exec_eisdir(struct file_tx* base, struct ofd_tx* ofd_tx,
                             int fildes, const void* buf, size_t nbyte,
                             bool isnoundo, int* cookie,
                             struct picotm_error* error);

void
file_tx_op_apply(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
                 int cookie, struct picotm_error* error);

void
file_tx_op_undo(struct file_tx* base, struct ofd_tx* ofd_tx, int sockfd,
                int cookie, struct picotm_error* error);
