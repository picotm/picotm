/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#pragma once

#include "picotm/picotm-error.h"
#include <sys/queue.h>
#include <sys/time.h>
#include "chrdev_tx.h"
#include "dir_tx.h"
#include "fdtab_tx.h"
#include "fd_tx.h"
#include "fifo_tx.h"
#include "fildes_event.h"
#include "ofd_tx.h"
#include "regfile_tx.h"
#include "socket_tx.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct fildes_log;
struct pipeop;
struct openop;

SLIST_HEAD(fd_tx_slist, fd_tx);
SLIST_HEAD(file_tx_slist, file_tx);
SLIST_HEAD(ofd_tx_slist, ofd_tx);

struct fildes_tx {

    struct fildes_log* log;

    struct fdtab_tx fdtab_tx;

    struct fd_tx fd_tx[MAXNUMFD];
    size_t       fd_tx_max_fildes;

    /** Active instances of |struct fd_tx| */
    struct fd_tx_slist  fd_tx_active_list;

    struct chrdev_tx chrdev_tx[MAXNUMFD];
    size_t           chrdev_tx_max_index;

    struct fifo_tx fifo_tx[MAXNUMFD];
    size_t         fifo_tx_max_index;

    struct regfile_tx regfile_tx[MAXNUMFD];
    size_t            regfile_tx_max_index;

    struct dir_tx dir_tx[MAXNUMFD];
    size_t        dir_tx_max_index;

    struct socket_tx  socket_tx[MAXNUMFD];
    size_t            socket_tx_max_index;

    /** Active instances of `struct file_tx` */
    struct file_tx_slist  file_tx_active_list;

    struct ofd_tx ofd_tx[MAXNUMFD];
    size_t        ofd_tx_max_index;

    /** Active instances of |struct ofd_tx| */
    struct ofd_tx_slist ofd_tx_active_list;

    struct openop* openoptab;
    size_t         openoptablen;

    struct pipeop* pipeoptab;
    size_t         pipeoptablen;
};

void
fildes_tx_init(struct fildes_tx* self, struct fildes_log* log);

void
fildes_tx_uninit(struct fildes_tx* self);

int
fildes_tx_exec_accept(struct fildes_tx* self, int sockfd,
                      struct sockaddr* address, socklen_t* address_len,
                      int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_bind(struct fildes_tx* self, int socket,
                    const struct sockaddr* address, socklen_t addresslen,
                    int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_chmod(struct fildes_tx* self, const char* path, mode_t mode,
                     struct picotm_error* error);

int
fildes_tx_exec_close(struct fildes_tx* self, int fildes, int isnoundo,
                     struct picotm_error* error);

int
fildes_tx_exec_connect(struct fildes_tx* self, int sockfd,
                       const struct sockaddr* serv_addr, socklen_t addrlen,
                       int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_dup(struct fildes_tx* self, int fildes, int cloexec,
                   int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_fchdir(struct fildes_tx* self, int fildes,
                      struct picotm_error* error);

int
fildes_tx_exec_fchmod(struct fildes_tx* self, int fildes, mode_t mode,
                      int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_fcntl(struct fildes_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, int isnoundo,
                     struct picotm_error* error);

int
fildes_tx_exec_fstat(struct fildes_tx* self, int fildes, struct stat* buf,
                     int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_fsync(struct fildes_tx* self, int fildes, int isnoundo,
                     struct picotm_error* error);

int
fildes_tx_exec_link(struct fildes_tx* self, const char* path1, const char* path2,
                    struct picotm_error* error);

int
fildes_tx_exec_listen(struct fildes_tx* self, int sockfd, int backlog,
                      int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_lstat(struct fildes_tx* self, const char* path, struct stat* buf,
                     struct picotm_error* error);

off_t
fildes_tx_exec_lseek(struct fildes_tx* self, int fildes, off_t offset,
                     int whence, int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_mkdir(struct fildes_tx* self, const char* path, mode_t mode,
                     struct picotm_error* error);

int
fildes_tx_exec_mkfifo(struct fildes_tx* self, const char* path, mode_t mode,
                      struct picotm_error* error);

int
fildes_tx_exec_mknod(struct fildes_tx* self, const char* path, mode_t mode,
                     dev_t dev, struct picotm_error* error);

int
fildes_tx_exec_mkstemp(struct fildes_tx* self, char* pathname,
                       struct picotm_error* error);

int
fildes_tx_exec_open(struct fildes_tx* self, const char* path, int oflag,
                    mode_t mode, int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_pipe(struct fildes_tx* self, int pipefd[2],
                    struct picotm_error* error);

ssize_t
fildes_tx_exec_pread(struct fildes_tx* self, int fildes, void* buf,
                     size_t nbyte, off_t off, int isnoundo,
                     struct picotm_error* error);

ssize_t
fildes_tx_exec_pwrite(struct fildes_tx* self, int fildes, const void* buf,
                      size_t nbyte, off_t off, int isnoundo,
                      struct picotm_error* error);

ssize_t
fildes_tx_exec_read(struct fildes_tx* self, int fildes, void* buf,
                    size_t nbyte, int isnoundo, struct picotm_error* error);

ssize_t
fildes_tx_exec_recv(struct fildes_tx* self, int sockfd, void* buffer,
                    size_t length, int flags, int isnoundo,
                    struct picotm_error* error);

int
fildes_tx_exec_select(struct fildes_tx* self, int nfds, fd_set* readfds,
                      fd_set* writefds, fd_set* errorfds,
                      struct timeval* timeout, int isnoundo,
                      struct picotm_error* error);

ssize_t
fildes_tx_exec_send(struct fildes_tx* self, int sockfd, const void* buffer,
                    size_t length, int flags, int isnoundo,
                    struct picotm_error* error);

int
fildes_tx_exec_shutdown(struct fildes_tx* self, int sockfd, int how,
                        int isnoundo, struct picotm_error* error);

int
fildes_tx_exec_socket(struct fildes_tx* self, int domain, int type,
                      int protocol, struct picotm_error* error);

int
fildes_tx_exec_stat(struct fildes_tx* self, const char* path, struct stat* buf,
                    struct picotm_error* error);

void
fildes_tx_exec_sync(struct fildes_tx* self, struct picotm_error* error);

int
fildes_tx_exec_unlink(struct fildes_tx* self, const char* path,
                      struct picotm_error* error);

ssize_t
fildes_tx_exec_write(struct fildes_tx* self, int fildes, const void* buf,
                     size_t nbyte, int isnoundo, struct picotm_error* error);

/*
 * Module interface
 */

void
fildes_tx_lock(struct fildes_tx* self, struct picotm_error* error);

void
fildes_tx_unlock(struct fildes_tx* self);

void
fildes_tx_validate(struct fildes_tx* self, int noundo,
                   struct picotm_error* error);

void
fildes_tx_apply_event(struct fildes_tx* self, enum fildes_op op, int fildes,
                      int cookie, struct picotm_error* error);

void
fildes_tx_undo_event(struct fildes_tx* self, enum fildes_op op, int fildes,
                     int cookie, struct picotm_error* error);

void
fildes_tx_update_cc(struct fildes_tx* self, int noundo,
                    struct picotm_error* error);

void
fildes_tx_clear_cc(struct fildes_tx* self, int noundo,
                   struct picotm_error* error);

void
fildes_tx_finish(struct fildes_tx* self, struct picotm_error* error);
