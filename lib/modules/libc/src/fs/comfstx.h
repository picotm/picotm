/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMFSTX_H
#define COMFSTX_H

#include <sys/types.h>

struct stat;

int
com_fs_tx_chdir(const char* path);

int
com_fs_tx_chmod(const char* path, mode_t mode);

int
com_fs_tx_fchdir(int fildes);

int
com_fs_tx_fchmod(int fildes, mode_t mode);

int
com_fs_tx_fstat(int fildes, struct stat* buf);

char*
com_fs_tx_getcwd(char* buf, size_t size);

int
com_fs_tx_getcwd_fildes(void);

int
com_fs_tx_link(const char* path1, const char* path2);

int
com_fs_tx_lstat(const char* path, struct stat* buf);

int
com_fs_tx_mkdir(const char* path, mode_t mode);

int
com_fs_tx_mkfifo(const char* path, mode_t mode);

int
com_fs_tx_mknod(const char* path, mode_t mode, dev_t dev);

int
com_fs_tx_mkstemp(char* template);

int
com_fs_tx_stat(const char* path, struct stat* buf);

int
com_fs_tx_unlink(const char* path);

#endif

