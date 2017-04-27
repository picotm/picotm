/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/sys/stat.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "error/module.h"
#include "fs/module.h"

PICOTM_EXPORT
int
chmod_tm(const char* path, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_chmod(path, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
fstat_tm(int fildes, struct stat* buf)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_fstat(fildes, buf);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
lstat_tm(const char* path, struct stat* buf)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_lstat(path, buf);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
mkdir_tm(const char* path, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_mkdir(path, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
mkfifo_tm(const char* path, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_mkfifo(path, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
PICOTM_EXPORT
int
mknod_tm(const char* path, mode_t mode, dev_t dev)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_mknod(path, mode, dev);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

PICOTM_EXPORT
int
stat_tm(const char* path, struct stat* buf)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_stat(path, buf);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
