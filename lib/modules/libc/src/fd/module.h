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

#include <sys/socket.h>
#include "fcntlop.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

int
fd_module_accept(int socket,
                 struct sockaddr* address,
                 socklen_t* address_len);

int
fd_module_bind(int socket,
               const struct sockaddr* address,
               socklen_t address_len);

int
fd_module_chdir(const char* path);

int
fd_module_chmod(const char* path, mode_t mode);

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
fd_module_fchdir(int fildes);

int
fd_module_fchmod(int fildes, mode_t mode);

int
fd_module_fcntl(int fildes, int cmd, union fcntl_arg* arg);

int
fd_module_fstat(int fildes, struct stat* buf);

int
fd_module_fsync(int fildes);

char*
fd_module_getcwd(char* buf, size_t size);

int
fd_module_link(const char* path1, const char* path2);

int
fd_module_listen(int socket, int backlog);

off_t
fd_module_lseek(int fildes, off_t offset, int whence);

int
fd_module_lstat(const char* path, struct stat* buf);

int
fd_module_mkdir(const char* path, mode_t mode);

int
fd_module_mkfifo(const char* path, mode_t mode);

int
fd_module_mknod(const char* path, mode_t mode, dev_t dev);

int
fd_module_mkstemp(char* template);

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

int
fd_module_stat(const char* path, struct stat* buf);

void
fd_module_sync(void);

int
fd_module_unlink(const char* path);

ssize_t
fd_module_write(int fildes, const void* buf, size_t nbyte);

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \defgroup libc_impl_fd libc File-Descriptor I/O
 * \endcond
 */
