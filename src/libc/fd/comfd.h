/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMFD_H
#define COMFD_H

#include <stdbool.h>

struct pipeop;
struct openop;
struct ofdtx;
struct fdtx;

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
com_fd_init(struct com_fd *comfd);

void
com_fd_uninit(struct com_fd *comfd);

void
com_fd_set_optcc(struct com_fd *comfd, int optcc);

int
com_fd_get_optcc(const struct com_fd *comfd);

void
com_fd_set_validation_mode(struct com_fd *comfd,
                           enum systx_libc_validation_mode val_mode);

enum systx_libc_validation_mode
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

#endif

