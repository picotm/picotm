/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <sys/socket.h>
#include "fcntlop.h"

int
fd_module_accept(int socket,
                 struct sockaddr* address,
                 socklen_t* address_len);

int
fd_module_bind(int socket,
               const struct sockaddr* address,
               socklen_t address_len);

int
fd_module_close(int fildes);

int
fd_module_connect(int socket,
                  const struct sockaddr* address,
                  socklen_t address_len);

int
fd_module_dup(int fildes);

int
fd_module_dup_internal(int fildes, int cloexec);

int
fd_module_fcntl(int fildes, int cmd, union com_fd_fcntl_arg* arg);

int
fd_module_fsync(int fildes);

int
fd_module_listen(int socket, int backlog);

off_t
fd_module_lseek(int fildes, off_t offset, int whence);

int
fd_module_open(const char* path, int oflag, mode_t mode);

int
fd_module_pipe(int fildes[2]);

ssize_t
fd_module_pread(int fildes, void* buf, size_t nbyte, off_t off);

ssize_t
fd_module_pwrite(int fildes, const void* buf, size_t nbyte, off_t off);

ssize_t
fd_module_read(int fildes, void* buf, size_t nbyte);

ssize_t
fd_module_recv(int socket, void* buffer, size_t length, int flags);

int
fd_module_select(int nfds,
                 fd_set* readfds, fd_set* writefds, fd_set* errorfds,
                 struct timeval* timeout);

ssize_t
fd_module_send(int socket, const void* buffer, size_t length, int flags);

int
fd_module_shutdown(int socket, int how);

int
fd_module_socket(int domain, int type, int protocol);

void
fd_module_sync(void);

ssize_t
fd_module_write(int fildes, const void* buf, size_t nbyte);
