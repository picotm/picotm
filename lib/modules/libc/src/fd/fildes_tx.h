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

#include <picotm/picotm-module.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include "chrdev_tx.h"
#include "dir_tx.h"
#include "fd_event.h"
#include "fd_tx.h"
#include "fifo_tx.h"
#include "regfile_tx.h"
#include "socket_tx.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct pipeop;
struct openop;

SLIST_HEAD(chrdev_tx_slist, chrdev_tx);
SLIST_HEAD(dir_tx_slist, dir_tx);
SLIST_HEAD(fd_tx_slist, fd_tx);
SLIST_HEAD(fifo_tx_slist, fifo_tx);
SLIST_HEAD(regfile_tx_slist, regfile_tx);
SLIST_HEAD(socket_tx_slist, socket_tx);

struct fildes_tx {
    unsigned long module;

    struct fd_tx fd_tx[MAXNUMFD];
    size_t       fd_tx_max_fildes;

    /** Active instances of |struct fd_tx| */
    struct fd_tx_slist  fd_tx_active_list;

    struct chrdev_tx chrdev_tx[MAXNUMFD];
    size_t           chrdev_tx_max_index;

    /** Active instances of |struct chrdev_tx| */
    struct chrdev_tx_slist    chrdev_tx_active_list;

    struct fifo_tx fifo_tx[MAXNUMFD];
    size_t         fifo_tx_max_index;

    /** Active instances of |struct fifo_tx| */
    struct fifo_tx_slist    fifo_tx_active_list;

    struct regfile_tx regfile_tx[MAXNUMFD];
    size_t            regfile_tx_max_index;

    /** Active instances of |struct regfile_tx| */
    struct regfile_tx_slist regfile_tx_active_list;

    struct dir_tx dir_tx[MAXNUMFD];
    size_t        dir_tx_max_index;

    /** Active instances of |struct dir_tx| */
    struct dir_tx_slist dir_tx_active_list;

    struct socket_tx  socket_tx[MAXNUMFD];
    size_t            socket_tx_max_index;

    /** Active instances of |struct socket_tx| */
    struct socket_tx_slist  socket_tx_active_list;

    /** File descriptor of initial working dir */
    int inicwd;
    /** File descriptor of current working dir */
    int newcwd;

    struct fd_event* eventtab;
    size_t           eventtablen;
    size_t           eventtabsiz;

    struct openop* openoptab;
    size_t         openoptablen;

    struct pipeop* pipeoptab;
    size_t         pipeoptablen;
};

void
fildes_tx_init(struct fildes_tx* self, unsigned long module);

void
fildes_tx_uninit(struct fildes_tx* self);

void
fildes_tx_set_validation_mode(struct fildes_tx* self,
                              enum picotm_libc_validation_mode val_mode);

enum picotm_libc_validation_mode
fildes_tx_get_validation_mode(const struct fildes_tx* self);

int
fildes_tx_exec_accept(struct fildes_tx* self, int sockfd,
                      struct sockaddr* address, socklen_t *address_len,
                      struct picotm_error* error);

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
                   struct picotm_error* error);

int
fildes_tx_exec_fchdir(struct fildes_tx* self, int fildes,
                      struct picotm_error* error);

int
fildes_tx_exec_fchmod(struct fildes_tx* self, int fildes, mode_t mode,
                      struct picotm_error* error);

int
fildes_tx_exec_fcntl(struct fildes_tx* self, int fildes, int cmd,
                     union fcntl_arg* arg, int isnoundo,
                     struct picotm_error* error);

int
fildes_tx_exec_fstat(struct fildes_tx* self, int fildes, struct stat* buf,
                     struct picotm_error* error);

int
fildes_tx_exec_fsync(struct fildes_tx* self, int fildes, int isnoundo,
                     struct picotm_error* error);

char*
fildes_tx_exec_getcwd(struct fildes_tx* self, char* buf, size_t size,
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
fildes_tx_apply_event(struct fildes_tx* self,
                      const struct picotm_event* event,
                      struct picotm_error* error);

void
fildes_tx_undo_event(struct fildes_tx* self,
                     const struct picotm_event* event,
                     struct picotm_error* error);

void
fildes_tx_update_cc(struct fildes_tx* self, int noundo,
                    struct picotm_error* error);

void
fildes_tx_clear_cc(struct fildes_tx* self, int noundo,
                   struct picotm_error* error);

void
fildes_tx_finish(struct fildes_tx* self, struct picotm_error* error);
