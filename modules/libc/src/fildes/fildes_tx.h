/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2020  Thomas Zimmermann
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

#pragma once

#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-slist.h"
#include <sys/time.h>
#include "fdtab_tx.h"
#include "fd_tx.h"
#include "fildes_event.h"

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

struct fildes_tx {

    struct fildes* fildes;

    struct fildes_log* log;

    struct fdtab_tx fdtab_tx;

    struct fd_tx fd_tx[MAXNUMFD];
    size_t       fd_tx_max_fildes;

    /** Active instances of |struct fd_tx| */
    struct picotm_slist fd_tx_active_list;

    /** Allocated instances of `struct file_tx` */
    struct picotm_slist  file_tx_alloced_list;

    /** Active instances of `struct file_tx` */
    struct picotm_slist  file_tx_active_list;

    /** Allocated instances of `struct pipebuf_tx` */
    struct picotm_slist  pipebuf_tx_alloced_list;

    /** Active instances of `struct pipebuf_tx` */
    struct picotm_slist  pipebuf_tx_active_list;

    /** Allocated instances of `struct seekbuf_tx` */
    struct picotm_slist  seekbuf_tx_alloced_list;

    /** Active instances of `struct seekbuf_tx` */
    struct picotm_slist  seekbuf_tx_active_list;

    /** Allocated instances of `struct sockbuf_tx` */
    struct picotm_slist  sockbuf_tx_alloced_list;

    /** Active instances of `struct sockbuf_tx` */
    struct picotm_slist  sockbuf_tx_active_list;

    struct openop* openoptab;
    size_t         openoptablen;

    struct pipeop* pipeoptab;
    size_t         pipeoptablen;
};

void
fildes_tx_init(struct fildes_tx* self, struct fildes* fildes,
               struct fildes_log* log);

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
fildes_tx_prepare_commit(struct fildes_tx* self, int noundo,
                         struct picotm_error* error);

void
fildes_tx_apply_event(struct fildes_tx* self, enum fildes_op op, int fildes,
                      int cookie, struct picotm_error* error);

void
fildes_tx_undo_event(struct fildes_tx* self, enum fildes_op op, int fildes,
                     int cookie, struct picotm_error* error);

void
fildes_tx_finish(struct fildes_tx* self, struct picotm_error* error);
