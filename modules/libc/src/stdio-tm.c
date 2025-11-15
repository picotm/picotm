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

#include "picotm/stdio-tm.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "error/module.h"

#if defined(PICOTM_LIBC_HAVE_SNPRINTF) && PICOTM_LIBC_HAVE_SNPRINTF
PICOTM_EXPORT
int
snprintf_tm(char* s, size_t n, const char* format, ...)
{
    va_list arg;

    va_start(arg, format);
    int res = vsnprintf_tm(s, n, format, arg);
    va_end(arg);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_SSCANF) && PICOTM_LIBC_HAVE_SSCANF
PICOTM_EXPORT
int
sscanf_tm(const char* s, const char* format, ...)
{
    va_list arg;

    va_start(arg, format);
    int res = vsscanf_tm(s, format, arg);
    va_end(arg);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_VSNPRINTF) && PICOTM_LIBC_HAVE_VSNPRINTF
PICOTM_EXPORT
int
vsnprintf_tm(char* s, size_t n, const char* format, va_list ap)
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
        int res = vsnprintf(s, n, format, ap);
        if (res < 0) {
            picotm_recover_from_errno(errno);
            continue;
        }
        return res;
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_VSSCANF) && PICOTM_LIBC_HAVE_VSSCANF
PICOTM_EXPORT
int
vsscanf_tm(const char* s, const char* format, va_list arg)
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
        int res = vsscanf(s, format, arg);
        if (res < 0) {
            picotm_recover_from_errno(errno);
            continue;
        }
        return res;
    } while (true);
}
#endif
