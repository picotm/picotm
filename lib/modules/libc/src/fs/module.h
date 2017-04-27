/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <sys/types.h>

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
