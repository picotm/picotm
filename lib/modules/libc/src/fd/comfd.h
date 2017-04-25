/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMFD_H
#define COMFD_H

#include <picotm/picotm-module.h>
#include <stdbool.h>
#include "ofdtx.h"
#include "fdtx.h"

struct pipeop;
struct openop;

enum com_fd_call
{
    ACTION_CLOSE = 0,
    ACTION_OPEN,
    ACTION_PREAD,
    ACTION_PWRITE,
    ACTION_LSEEK,
    ACTION_READ,
    ACTION_WRITE,
    ACTION_FCNTL,
    ACTION_FSYNC,
    ACTION_SYNC,
    ACTION_DUP,
    ACTION_PIPE,
    /* Socket calls */
    ACTION_SOCKET,
    ACTION_LISTEN,
    ACTION_CONNECT,
    ACTION_ACCEPT,
    ACTION_SEND,
    ACTION_RECV,
    ACTION_SHUTDOWN,
    ACTION_BIND
};

struct com_fd_event
{
    int fildes;
    int cookie;
};

struct com_fd
{
    unsigned long module;

    bool optcc;

    struct ofdtx ofdtx[MAXNUMFD];
    struct fdtx   fdtx[MAXNUMFD];

    size_t ofdtx_max_index;
    size_t fdtx_max_fildes;

    struct com_fd_event *eventtab;
    size_t               eventtablen;
    size_t               eventtabsiz;

    struct openop   *openoptab;
    size_t           openoptablen;

    struct pipeop   *pipeoptab;
    size_t           pipeoptablen;

    /* Locked fds and ofds during commit */
    int   *ifd;
    size_t ifdlen;
    int   *iofd;
    size_t iofdlen;
};

int
com_fd_init(struct com_fd *comfd, unsigned long module);

void
com_fd_uninit(struct com_fd *comfd);

void
com_fd_set_optcc(struct com_fd *comfd, int optcc);

int
com_fd_get_optcc(const struct com_fd *comfd);

void
com_fd_set_validation_mode(struct com_fd *comfd,
                           enum picotm_libc_validation_mode val_mode);

enum picotm_libc_validation_mode
com_fd_get_validation_mode(const struct com_fd *comfd);

/** \brief Append an fd action to log */
int
com_fd_inject(struct com_fd *comfd, enum com_fd_call call, int fildes, int cookie);

/** \brief Get transaction-local state for file descriptor */
struct fdtx *
com_fd_get_fdtx(struct com_fd *comfd, int fildes);

/** \brief Get transaction-local state for open file description */
struct ofdtx *
com_fd_get_ofdtx(struct com_fd *comfd, int index);

int
com_fd_lock(struct com_fd *comfd);

void
com_fd_unlock(struct com_fd *comfd);

int
com_fd_validate(struct com_fd *comfd, int noundo);

int
com_fd_apply_event(struct com_fd *comfd, const struct event *event, size_t n);

int
com_fd_undo_event(struct com_fd *comfd, const struct event *event, size_t n);

int
com_fd_updatecc(struct com_fd *comfd, int noundo);

int
com_fd_clearcc(struct com_fd *comfd, int noundo);

void
com_fd_finish(struct com_fd *comfd);

/*
 * accept()
 */

int
com_fd_exec_accept(struct com_fd *data, int sockfd, struct sockaddr *address,
                   socklen_t *address_len);

int
com_fd_apply_accept(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_accept(struct com_fd *data, int fildes, int cookie);

/*
 * bind()
 */

int
com_fd_exec_bind(struct com_fd *data, int socket, const struct sockaddr *address,
                                              socklen_t addresslen, int isnoundo);

int
com_fd_apply_bind(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_bind(struct com_fd *data, int fildes, int cookie);

/*
 * close()
 */

int
com_fd_exec_close(struct com_fd *data, int fildes, int isnoundo);

int
com_fd_apply_close(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_close(struct com_fd *data, int fildes, int cookie);

/*
 * connect()
 */

int
com_fd_exec_connect(struct com_fd *data, int sockfd,
                    const struct sockaddr *serv_addr, socklen_t addrlen,
                    int isnoundo);

int
com_fd_apply_connect(struct com_fd *data, const struct com_fd_event *event,
                     size_t n);

int
com_fd_undo_connect(struct com_fd *data, int fildes, int cookie);

/*
 * dup()
 */

int
com_fd_exec_dup(struct com_fd *data, int fildes, int cloexec);

int
com_fd_apply_dup(struct com_fd *data, const struct com_fd_event *event,
                 size_t n);

int
com_fd_undo_dup(struct com_fd *data, int fildes, int cookie);

/*
 * fcntl()
 */

int
com_fd_exec_fcntl(struct com_fd *data, int fildes, int cmd,
                  union com_fd_fcntl_arg *arg, int isnoundo);

int
com_fd_apply_fcntl(struct com_fd *data, const struct com_fd_event *event,
                   size_t n);

int
com_fd_undo_fcntl(struct com_fd *data, int fildes, int cookie);

/*
 * fcntl()
 */

int
com_fd_exec_fsync(struct com_fd *data, int fildes, int isnoundo);

int
com_fd_apply_fsync(struct com_fd *data, const struct com_fd_event *event,
                   size_t n);

int
com_fd_undo_fsync(struct com_fd *data, int fildes, int cookie);

/*
 * listen()
 */

int
com_fd_exec_listen(struct com_fd *data, int sockfd, int backlog,
                   int isnoundo);

int
com_fd_apply_listen(struct com_fd *data, const struct com_fd_event *event,
                    size_t n);

int
com_fd_undo_listen(struct com_fd *data, int fildes, int cookie);

/*
 * lseek()
 */

off_t
com_fd_exec_lseek(struct com_fd *data, int fildes, off_t offset, int whence,
                  int isnoundo);

int
com_fd_apply_lseek(struct com_fd *data, const struct com_fd_event *event,
                   size_t n);

int
com_fd_undo_lseek(struct com_fd *data, int fildes, int cookie);

/*
 * open()
 */

int
com_fd_exec_open(struct com_fd *data, const char *path, int oflag,
                                                        mode_t mode,
                                                        int isnoundo);

int
com_fd_apply_open(struct com_fd *data, const struct com_fd_event *event,
                  size_t n);

int
com_fd_undo_open(struct com_fd *data, int fildes, int cookie);

/*
 * pipe()
 */

int
com_fd_exec_pipe(struct com_fd *data, int pipefd[2]);

int
com_fd_apply_pipe(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_pipe(struct com_fd *data, int fildes, int cookie);

/*
 * pread()
 */

ssize_t
com_fd_exec_pread(struct com_fd *data, int fildes, void *buf, size_t nbyte,
                  off_t off, int isnoundo);

int
com_fd_apply_pread(struct com_fd *data, const struct com_fd_event *event,
                   size_t n);

int
com_fd_undo_pread(struct com_fd *data, int fildes, int cookie);

/*
 * pwrite()
 */

ssize_t
com_fd_exec_pwrite(struct com_fd *data, int fildes, const void *buf,
                   size_t nbyte, off_t off, int isnoundo);

int
com_fd_apply_pwrite(struct com_fd *data, const struct com_fd_event *event,
                    size_t n);

int
com_fd_undo_pwrite(struct com_fd *data, int fildes, int cookie);

/*
 * read()
 */

ssize_t
com_fd_exec_read(struct com_fd *data, int fildes, void *buf, size_t nbyte,
                 int isnoundo);

int
com_fd_apply_read(struct com_fd *data, const struct com_fd_event *event,
                  size_t n);

int
com_fd_undo_read(struct com_fd *data, int fildes, int cookie);

/*
 * recv()
 */

ssize_t
com_fd_exec_recv(struct com_fd *data, int sockfd, void *buffer,
                 size_t length, int flags, int isnoundo);

int
com_fd_apply_recv(struct com_fd *data, const struct com_fd_event *event,
                  size_t n);

int
com_fd_undo_recv(struct com_fd *data, int fildes, int cookie);

/*
 * select()
 */

int
com_fd_exec_select(struct com_fd *data, int nfds, fd_set *readfds,
                                                  fd_set *writefds,
                                                  fd_set *errorfds,
                                                  struct timeval *timeout,
                                                  int isnoundo);

/*
 * send()
 */

ssize_t
com_fd_exec_send(struct com_fd *data, int sockfd, const void *buffer,
                 size_t length, int flags, int isnoundo);

int
com_fd_apply_send(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_send(struct com_fd *data, int fildes, int cookie);

/*
 * shutdown()
 */

int
com_fd_exec_shutdown(struct com_fd *data, int sockfd, int how, int isnoundo);

int
com_fd_apply_shutdown(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_shutdown(struct com_fd *data, int fildes, int cookie);

/*
 * socket()
 */

int
com_fd_exec_socket(struct com_fd *data, int domain, int type, int protocol);

int
com_fd_apply_socket(struct com_fd *data, const struct com_fd_event *event,
                    size_t n);

int
com_fd_undo_socket(struct com_fd *data, int fildes, int cookie);

/*
 * sync()
 */

void
com_fd_exec_sync(struct com_fd *data);

int
com_fd_apply_sync(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_sync(struct com_fd *data, int fildes, int cookie);

/*
 * write()
 */

ssize_t
com_fd_exec_write(struct com_fd *data, int fildes, const void *buf, size_t nbyte, int isnoundo);

int
com_fd_apply_write(struct com_fd *data, const struct com_fd_event *event, size_t n);

int
com_fd_undo_write(struct com_fd *data, int fildes, int cookie);

#endif

