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

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_vfs
 * \ingroup libc_impl
 * \ingroup libc_impl_vfs
 * \file
 * \endcond
 */

struct stat;

int
vfs_module_chdir(const char* path);

int
vfs_module_chmod(const char* path, mode_t mode);

int
vfs_module_fchdir(int fildes);

int
vfs_module_fchmod(int fildes, mode_t mode);

int
vfs_module_fstat(int fildes, struct stat* buf);

char*
vfs_module_getcwd(char* buf, size_t size);

int
vfs_module_getcwd_fildes(void);

int
vfs_module_link(const char* path1, const char* path2);

int
vfs_module_lstat(const char* path, struct stat* buf);

int
vfs_module_mkdir(const char* path, mode_t mode);

int
vfs_module_mkfifo(const char* path, mode_t mode);

int
vfs_module_mknod(const char* path, mode_t mode, dev_t dev);

int
vfs_module_mkstemp(char* template);

int
vfs_module_stat(const char* path, struct stat* buf);

int
vfs_module_unlink(const char* path);

/**
 * \cond impl || libc_impl || libc_impl_vfs
 * \defgroup libc_impl_vfs libc File-System Support
 * \endcond
 */
