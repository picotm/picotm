/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/picotm-module.h>
#include <stdbool.h>
#include "fd_tx.h"
#include "ofd_tx.h"
#include "fd_event.h"

struct pipeop;
struct openop;

struct fildes_tx {
    unsigned long module;

    bool optcc;

    struct ofd_tx ofd_tx[MAXNUMFD];
    struct fd_tx  fd_tx[MAXNUMFD];

    size_t ofd_tx_max_index;
    size_t fd_tx_max_fildes;

    struct fd_event* eventtab;
    size_t           eventtablen;
    size_t           eventtabsiz;

    struct openop* openoptab;
    size_t         openoptablen;

    struct pipeop* pipeoptab;
    size_t         pipeoptablen;

    /* Locked fds and ofds during commit */
    int*   ifd;
    size_t ifdlen;
    int*   iofd;
    size_t iofdlen;
};

int
fildes_tx_init(struct fildes_tx* self, unsigned long module);

void
fildes_tx_uninit(struct fildes_tx* self);

void
fildes_tx_set_optcc(struct fildes_tx* self, int optcc);

int
fildes_tx_get_optcc(const struct fildes_tx* self);

void
fildes_tx_set_validation_mode(struct fildes_tx* self,
                              enum picotm_libc_validation_mode val_mode);

enum picotm_libc_validation_mode
fildes_tx_get_validation_mode(const struct fildes_tx* self);

int
fildes_tx_exec_accept(struct fildes_tx* self, int sockfd,
                      struct sockaddr* address, socklen_t *address_len);

int
fildes_tx_exec_bind(struct fildes_tx* self, int socket,
                    const struct sockaddr* address, socklen_t addresslen,
                    int isnoundo);

int
fildes_tx_exec_close(struct fildes_tx* self, int fildes, int isnoundo);

int
fildes_tx_exec_connect(struct fildes_tx* self, int sockfd,
                       const struct sockaddr* serv_addr, socklen_t addrlen,
                       int isnoundo);

int
fildes_tx_exec_dup(struct fildes_tx* self, int fildes, int cloexec);

int
fildes_tx_exec_fcntl(struct fildes_tx* self, int fildes, int cmd,
                     union com_fd_fcntl_arg* arg, int isnoundo);

int
fildes_tx_exec_fsync(struct fildes_tx* self, int fildes, int isnoundo);

int
fildes_tx_exec_listen(struct fildes_tx* self, int sockfd, int backlog,
                      int isnoundo);

off_t
fildes_tx_exec_lseek(struct fildes_tx* self, int fildes, off_t offset,
                     int whence, int isnoundo);

int
fildes_tx_exec_open(struct fildes_tx* self, const char* path, int oflag,
                    mode_t mode, int isnoundo);

int
fildes_tx_exec_pipe(struct fildes_tx* self, int pipefd[2]);

ssize_t
fildes_tx_exec_pread(struct fildes_tx* self, int fildes, void* buf,
                     size_t nbyte, off_t off, int isnoundo);

ssize_t
fildes_tx_exec_pwrite(struct fildes_tx* self, int fildes, const void* buf,
                      size_t nbyte, off_t off, int isnoundo);

ssize_t
fildes_tx_exec_read(struct fildes_tx* self, int fildes, void* buf,
                    size_t nbyte, int isnoundo);

ssize_t
fildes_tx_exec_recv(struct fildes_tx* self, int sockfd, void* buffer,
                    size_t length, int flags, int isnoundo);

int
fildes_tx_exec_select(struct fildes_tx* self, int nfds, fd_set* readfds,
                      fd_set* writefds, fd_set* errorfds,
                      struct timeval* timeout, int isnoundo);

ssize_t
fildes_tx_exec_send(struct fildes_tx* self, int sockfd, const void* buffer,
                    size_t length, int flags, int isnoundo);

int
fildes_tx_exec_shutdown(struct fildes_tx* self, int sockfd, int how,
                        int isnoundo);

int
fildes_tx_exec_socket(struct fildes_tx* self, int domain, int type,
                      int protocol);

void
fildes_tx_exec_sync(struct fildes_tx* self);

ssize_t
fildes_tx_exec_write(struct fildes_tx* self, int fildes, const void* buf,
                     size_t nbyte, int isnoundo);

int
fildes_tx_lock(struct fildes_tx* self);

void
fildes_tx_unlock(struct fildes_tx* self);

int
fildes_tx_validate(struct fildes_tx* self, int noundo);

int
fildes_tx_apply_event(struct fildes_tx* self, const struct event* event,
                      size_t nevents);

int
fildes_tx_undo_event(struct fildes_tx* self, const struct event* event,
                     size_t nevents);

int
fildes_tx_update_cc(struct fildes_tx* self, int noundo);

int
fildes_tx_clear_cc(struct fildes_tx* self, int noundo);

void
fildes_tx_finish(struct fildes_tx* self);
