/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/sys/stat.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "fs/comfstx.h"
#include "picotm/picotm-libc.h"

PICOTM_EXPORT
int
chmod_tm(const char* path, mode_t mode)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_chmod(path, mode);
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
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_fstat(fildes, buf);
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
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_lstat(path, buf);
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
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_mkdir(path, mode);
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
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_mkfifo(path, mode);
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
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_mknod(path, mode, dev);
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
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fs_tx_stat(path, buf);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
