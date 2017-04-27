/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/sys/stat.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include "error/module.h"
#include "fs/module.h"
#include "picotm/sys/stat-tm.h"

PICOTM_EXPORT
int
chmod_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return chmod_tm(path, mode);
}

PICOTM_EXPORT
int
fchmod_tx(int fildes, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_fchmod(fildes, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
fstat_tx(int fildes, struct stat* buf)
{
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return fstat_tm(fildes, buf);
}

PICOTM_EXPORT
int
lstat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return lstat_tm(path, buf);
}

PICOTM_EXPORT
int
mkdir_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mkdir_tm(path, mode);
}

PICOTM_EXPORT
int
mkfifo_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mkfifo_tm(path, mode);
}

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
PICOTM_EXPORT
int
mknod_tx(const char* path, mode_t mode, dev_t dev)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mknod_tm(path, mode, dev);
}
#endif

PICOTM_EXPORT
int
stat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return stat_tm(path, buf);
}
