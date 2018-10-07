/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "fcntlop.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

int
fildes_module_accept(int socket,
                     struct sockaddr* address,
                     socklen_t* address_len);

int
fildes_module_bind(int socket,
                   const struct sockaddr* address,
                   socklen_t address_len);

int
fildes_module_chmod(const char* path, mode_t mode);

int
fildes_module_close(int fildes);

int
fildes_module_connect(int socket,
                      const struct sockaddr* address,
                      socklen_t address_len);

int
fildes_module_dup(int fildes);

int
fildes_module_dup_internal(int fildes, int cloexec);

int
fildes_module_fchdir(int fildes);

int
fildes_module_fchmod(int fildes, mode_t mode);

int
fildes_module_fcntl(int fildes, int cmd, union fcntl_arg* arg);

int
fildes_module_fstat(int fildes, struct stat* buf);

int
fildes_module_fsync(int fildes);

int
fildes_module_link(const char* path1, const char* path2);

int
fildes_module_listen(int socket, int backlog);

off_t
fildes_module_lseek(int fildes, off_t offset, int whence);

int
fildes_module_lstat(const char* path, struct stat* buf);

int
fildes_module_mkdir(const char* path, mode_t mode);

int
fildes_module_mkfifo(const char* path, mode_t mode);

int
fildes_module_mknod(const char* path, mode_t mode, dev_t dev);

int
fildes_module_mkstemp(char* template);

int
fildes_module_open(const char* path, int oflag, mode_t mode);

int
fildes_module_pipe(int fildes[2]);

ssize_t
fildes_module_pread(int fildes, void* buf, size_t nbyte, off_t off);

ssize_t
fildes_module_pwrite(int fildes, const void* buf, size_t nbyte, off_t off);

ssize_t
fildes_module_read(int fildes, void* buf, size_t nbyte);

ssize_t
fildes_module_recv(int socket, void* buffer, size_t length, int flags);

int
fildes_module_select(int nfds,
                 fd_set* readfds, fd_set* writefds, fd_set* errorfds,
                 struct timeval* timeout);

ssize_t
fildes_module_send(int socket, const void* buffer, size_t length, int flags);

int
fildes_module_shutdown(int socket, int how);

int
fildes_module_socket(int domain, int type, int protocol);

int
fildes_module_stat(const char* path, struct stat* buf);

void
fildes_module_sync(void);

int
fildes_module_unlink(const char* path);

ssize_t
fildes_module_write(int fildes, const void* buf, size_t nbyte);

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \defgroup libc_impl_fd libc File-Descriptor I/O
 * \endcond
 */
