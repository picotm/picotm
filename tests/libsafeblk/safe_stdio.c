/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "safe_stdio.h"
#include <errno.h>
#include "safeblk.h"
#include "taputils.h"

/*
 * Interfaces with variable number of arguments
 */

int
safe_snprintf(char* str, size_t size, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = safe_vsnprintf(str, size, format, ap);
    va_end(ap);

    return res;
}

int
safe_sscanf(const char* str, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = safe_vsscanf(str, format, ap);
    va_end(ap);

    return res;
}

/*
 * Interfaces with va_list argument
 */

int
safe_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    int res = vsnprintf(str, size, format, ap);
    if (res < 0) {
        tap_error_errno("vsnprintf()", errno);
        abort_safe_block();
    } else if ((size_t)res >= size) {
        tap_error("vsnprintf() output truncated\n");
        abort_safe_block();
    }
    return res;
}

int
safe_vsscanf(const char* str, const char* format, va_list ap)
{
    int res = vsscanf(str, format, ap);
    if (res < 0) {
        tap_error_errno("vsscanf()", errno);
        abort_safe_block();
    }
    return res;
}
