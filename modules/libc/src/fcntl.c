/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "picotm/fcntl.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-tm.h"
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "compat/static_assert.h"
#include "error/module.h"
#include "fildes/module.h"
#include "picotm/fcntl-tm.h"

#if defined(PICOTM_LIBC_HAVE_CREAT) && PICOTM_LIBC_HAVE_CREAT
PICOTM_EXPORT
int
creat_tx(const char* path, mode_t mode)
{
    return open_tx(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}
#endif

#if defined(PICOTM_LIBC_HAVE_FCNTL) && PICOTM_LIBC_HAVE_FCNTL
PICOTM_EXPORT
int
fcntl_tx(int fildes, int cmd, ...)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        error_module_save_errno(&error);
        if (!picotm_error_is_set(&error)) {
            break;
        }
        picotm_recover_from_error(&error);
    } while (true);

    do {
        int res;
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        switch (cmd) {
            case F_DUPFD:
                /* Handle like dup() */
                res = fildes_module_dup_internal(fildes, false, &error);
                break;
#if defined(F_DUPFD_CLOEXEC)
            case F_DUPFD_CLOEXEC:
                /* Handle like dup() with CLOEXEC */
                res = fildes_module_dup_internal(fildes, true, &error);
                break;
#endif
            case F_SETFD:
            case F_SETFL:
            case F_SETOWN: {
                union fcntl_arg val;
                va_list arg;
                va_start(arg, cmd);
                val.arg0 = va_arg(arg, int);
                va_end(arg);

                res = fildes_module_fcntl(fildes, cmd, &val, &error);
                break;
            }
            case F_GETLK:
            case F_SETLK:
            case F_SETLKW: {
                union fcntl_arg val;
                struct flock* f;
                va_list arg;
                va_start(arg, cmd);
                f = va_arg(arg, struct flock*);
                va_end(arg);

                privatize_tx(f, sizeof(val.arg1), PICOTM_TM_PRIVATIZE_LOAD);
                memcpy(&val.arg1, f, sizeof(val.arg1));

                res = fildes_module_fcntl(fildes, cmd, &val, &error);
                break;
            }
            default:
                res = fildes_module_fcntl(fildes, cmd, NULL, &error);
                break;
        }
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_OPEN) && PICOTM_LIBC_HAVE_OPEN
PICOTM_EXPORT
int
open_tx(const char* path, int oflag, ...)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);

    if (!(oflag & O_CREAT)) {
        return open_tm(path, oflag);
    }

    /* The optional third argument is of type `mode_t`. It's
     * decoded as `int`. If `mode_t` has a conversion rank
     * larger than `int`, we have to adapt the call to
     * va_arg(). */
    static_assert(sizeof(mode_t) <= sizeof(int),
                  "`mode_t` has incorrect size");

    va_list arg;
    va_start(arg, oflag);
    mode_t mode = va_arg(arg, int);
    va_end(arg);

    return open_tm(path, oflag, mode);
}
#endif
