/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
    error_module_save_errno();

    int res;

    do {
        switch (cmd) {
            case F_DUPFD:
                /* Handle like dup() */
                res = fildes_module_dup_internal(fildes, false);
                break;
#if defined(F_DUPFD_CLOEXEC)
            case F_DUPFD_CLOEXEC:
                /* Handle like dup() with CLOEXEC */
                res = fildes_module_dup_internal(fildes, true);
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

                res = fildes_module_fcntl(fildes, cmd, &val);
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

                res = fildes_module_fcntl(fildes, cmd, &val);
                break;
            }
            default:
                res = fildes_module_fcntl(fildes, cmd, NULL);
                break;
        }
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
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
