/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/sys/stat.h"
#include <systx/systx-tm.h>
#include "fs/comfstx.h"
#include "systx/sys/stat-tm.h"

SYSTX_EXPORT
int
chmod_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return chmod_tm(path, mode);
}

SYSTX_EXPORT
int
fchmod_tx(int fildes, mode_t mode)
{
    return com_fs_tx_fchmod(fildes, mode);
}

SYSTX_EXPORT
int
fstat_tx(int fildes, struct stat* buf)
{
    privatize_tx(buf, sizeof(*buf), SYSTX_TM_PRIVATIZE_STORE);
    return fstat_tm(fildes, buf);
}

SYSTX_EXPORT
int
lstat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), SYSTX_TM_PRIVATIZE_STORE);
    return lstat_tm(path, buf);
}

SYSTX_EXPORT
int
mkdir_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return mkdir_tm(path, mode);
}

SYSTX_EXPORT
int
mkfifo_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return mkfifo_tm(path, mode);
}

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
SYSTX_EXPORT
int
mknod_tx(const char* path, mode_t mode, dev_t dev)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return mknod_tm(path, mode, dev);
}
#endif

SYSTX_EXPORT
int
stat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), SYSTX_TM_PRIVATIZE_STORE);
    return stat_tm(path, buf);
}
