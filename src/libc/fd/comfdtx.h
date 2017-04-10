/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMFDTX_H
#define COMFDTX_H

#include <fcntl.h>
#include <sys/socket.h>
#include "fcntlop.h"

struct com_fd;

/** \brief Aquire component data */
struct com_fd *
com_fd_tx_aquire_data(void);

int
com_fd_tx_accept(int socket,
                 struct sockaddr* address,
                 socklen_t* address_len);

int
com_fd_tx_bind(int socket,
               const struct sockaddr* address,
               socklen_t address_len);

int
com_fd_tx_close(int fildes);

int
com_fd_tx_connect(int socket,
                  const struct sockaddr* address,
                  socklen_t address_len);

int
com_fd_tx_dup(int fildes);

int
com_fd_tx_fcntl(int fildes, int cmd, union com_fd_fcntl_arg* arg);

int
com_fd_tx_fsync(int fildes);

int
com_fd_tx_listen(int socket, int backlog);

off_t
com_fd_tx_lseek(int fildes, off_t offset, int whence);

int
com_fd_tx_open(const char* path, int oflag, mode_t mode);

int
com_fd_tx_pipe(int fildes[2]);

ssize_t
com_fd_tx_pread(int fildes, void* buf, size_t nbyte, off_t off);

ssize_t
com_fd_tx_pwrite(int fildes, const void* buf, size_t nbyte, off_t off);

ssize_t
com_fd_tx_read(int fildes, void* buf, size_t nbyte);

ssize_t
com_fd_tx_recv(int socket, void* buffer, size_t length, int flags);

int
com_fd_tx_select(int nfds,
                 fd_set* readfds, fd_set* writefds, fd_set* errorfds,
                 struct timeval* timeout);

ssize_t
com_fd_tx_send(int socket, const void* buffer, size_t length, int flags);

int
com_fd_tx_shutdown(int socket, int how);

int
com_fd_tx_socket(int domain, int type, int protocol);

void
com_fd_tx_sync(void);

ssize_t
com_fd_tx_write(int fildes, const void* buf, size_t nbyte);

#endif
