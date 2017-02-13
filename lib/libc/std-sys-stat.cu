/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

ceuta_hdrl(#ifndef TANGER_STM_STD_SYS_STAT_H);
ceuta_hdrl(#define TANGER_STM_STD_SYS_STAT_H);
ceuta_hdrl(#include <sys/stat.h>);

#include <features.h>
#include <sys/stat.h>

extern int com_fs_tx_chmod(const char*, mode_t);
extern int com_fs_tx_fchmod(int, mode_t);
extern int com_fs_tx_fstat(int, struct stat*);
extern int com_fs_tx_lstat(const char*, struct stat*);
extern int com_fs_tx_mkdir(const char*, mode_t);
extern int com_fs_tx_mkfifo(const char*, mode_t);
extern int com_fs_tx_mknod(const char*, mode_t, dev_t);
extern int com_fs_tx_stat(const char*, struct stat*);

ceuta_wrap(int,    chmod,  com_fs_tx_chmod,  [in|cstring] const char *path, mode_t mode);
ceuta_wrap(int,    fchmod, com_fs_tx_fchmod,                    int fildes, mode_t mode);
ceuta_wrap(int,    fstat,  com_fs_tx_fstat,                     int fildes, [out] struct stat *buf);
ceuta_wrap(int,    lstat,  com_fs_tx_lstat,  [in|cstring] const char *path, [out] struct stat *buf);
ceuta_wrap(int,    mkdir,  com_fs_tx_mkdir,  [in|cstring] const char *path, mode_t mode);
ceuta_wrap(int,    mkfifo, com_fs_tx_mkfifo, [in|cstring] const char *path, mode_t mode);

ceuta_hdrl(#ifdef _BSD_SOURCE || _SVID_SOURCE || _XOPEN_SOURCE >= 500);
ceuta_wrap(int,    mknod,  com_fs_tx_mknod,  [in|cstring] const char *path, mode_t mode, dev_t dev);
ceuta_hdrl(#endif);

ceuta_wrap(int,    stat,   com_fs_tx_stat,   [in|cstring] const char *path, [out] struct stat *buf);
ceuta_pure(mode_t, umask,  umask,                              mode_t mode);

ceuta_hdrl(#endif);

