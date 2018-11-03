/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/sys/stat.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-tm.h"
#include "picotm/sys/stat-tm.h"
#include <errno.h>
#include "error/module.h"
#include "fildes/module.h"

#if defined(PICOTM_LIBC_HAVE_CHMOD) && PICOTM_LIBC_HAVE_CHMOD
PICOTM_EXPORT
int
chmod_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return chmod_tm(path, mode);
}
#endif

#if defined(PICOTM_LIBC_HAVE_FCHMOD) && PICOTM_LIBC_HAVE_FCHMOD
PICOTM_EXPORT
int
fchmod_tx(int fildes, mode_t mode)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        int res = fildes_module_fchmod(fildes, mode, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_FSTAT) && PICOTM_LIBC_HAVE_FSTAT
PICOTM_EXPORT
int
fstat_tx(int fildes, struct stat* buf)
{
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return fstat_tm(fildes, buf);
}
#endif

#if defined(PICOTM_LIBC_HAVE_LSTAT) && PICOTM_LIBC_HAVE_LSTAT
PICOTM_EXPORT
int
lstat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return lstat_tm(path, buf);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MKDIR) && PICOTM_LIBC_HAVE_MKDIR
PICOTM_EXPORT
int
mkdir_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mkdir_tm(path, mode);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MKFIFO) && PICOTM_LIBC_HAVE_MKFIFO
PICOTM_EXPORT
int
mkfifo_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mkfifo_tm(path, mode);
}
#endif

#if defined(PICOTM_LIBC_HAVE_MKNOD) && PICOTM_LIBC_HAVE_MKNOD
#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
PICOTM_EXPORT
int
mknod_tx(const char* path, mode_t mode, dev_t dev)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mknod_tm(path, mode, dev);
}
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_STAT) && PICOTM_LIBC_HAVE_STAT
PICOTM_EXPORT
int
stat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return stat_tm(path, buf);
}
#endif
