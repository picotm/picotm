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

#include "picotm/unistd-tm.h"
#include "picotm/picotm.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "cwd/module.h"
#include "error/module.h"
#include "fildes/module.h"

static bool
perform_recovery(int errno_hint)
{
    enum picotm_libc_error_recovery recovery =
        picotm_libc_get_error_recovery();

    if (recovery == PICOTM_LIBC_ERROR_RECOVERY_FULL) {
        return true;
    }

    return (errno_hint != EAGAIN) && (errno_hint != EWOULDBLOCK);
}

#if defined(PICOTM_LIBC_HAVE_CHDIR) && PICOTM_LIBC_HAVE_CHDIR
PICOTM_EXPORT
int
chdir_tm(const char* path)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = cwd_module_chdir(path, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_GETCWD) && PICOTM_LIBC_HAVE_GETCWD
PICOTM_EXPORT
char*
getcwd_tm(char* buf, size_t size)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        char* cwd = cwd_module_getcwd(buf, size, &error);
        if (!picotm_error_is_set(&error)) {
            return cwd;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_LINK) && PICOTM_LIBC_HAVE_LINK
PICOTM_EXPORT
int
link_tm(const char* path1, const char* path2)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_link(path1, path2);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_MKDTEMP) && PICOTM_LIBC_HAVE_MKDTEMP && \
    defined(__MACH__)
PICOTM_EXPORT
char*
mkdtemp_tm(char* template)
{
    error_module_save_errno();

    char* str;

    do {
        str = mkdtemp(template);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);

    return str;
}
#endif

#if defined(PICOTM_LIBC_HAVE_PIPE) && PICOTM_LIBC_HAVE_PIPE
PICOTM_EXPORT
int
pipe_tm(int fildes[2])
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_pipe(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_PREAD) && PICOTM_LIBC_HAVE_PREAD
PICOTM_EXPORT
ssize_t
pread_tm(int fildes, void* buf, size_t nbyte, off_t offset)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_pread(fildes, buf, nbyte, offset);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_PWRITE) && PICOTM_LIBC_HAVE_PWRITE
PICOTM_EXPORT
ssize_t
pwrite_tm(int fildes, const void* buf, size_t nbyte, off_t offset)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_pwrite(fildes, buf, nbyte, offset);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_READ) && PICOTM_LIBC_HAVE_READ
PICOTM_EXPORT
ssize_t
read_tm(int fildes, void* buf, size_t nbyte)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_read(fildes, buf, nbyte);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_UNLINK) && PICOTM_LIBC_HAVE_UNLINK
PICOTM_EXPORT
int
unlink_tm(const char* path)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_unlink(path);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_WRITE) && PICOTM_LIBC_HAVE_WRITE
PICOTM_EXPORT
ssize_t
write_tm(int fildes, const void* buf, size_t nbyte)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_write(fildes, buf, nbyte);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif
