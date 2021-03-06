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

#include "picotm/stdio.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-tm.h"
#include "picotm/stdio-tm.h"
#include <errno.h>
#include <string.h>
#include "error/module.h"

#if defined(PICOTM_LIBC_HAVE_SNPRINTF) && PICOTM_LIBC_HAVE_SNPRINTF
PICOTM_EXPORT
int
snprintf_tx(char* s, size_t n, const char* format, ...)
{
    va_list arg;

    va_start(arg, format);
    int res = vsnprintf_tx(s, n, format, arg);
    va_end(arg);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_SSCANF) && PICOTM_LIBC_HAVE_SSCANF
PICOTM_EXPORT
int
sscanf_tx(const char* s, const char* format, ...)
{
    va_list arg;

    va_start(arg, format);
    int res = vsscanf_tx(s, format, arg);
    va_end(arg);

    return res;
}
#endif

static void
privatize_printf_args(const char* format, va_list arg)
{
    while (format) {

        const char* pos = strchr(format, '%');
        if (!pos) {
            return;
        }

        switch (pos[1]) {

            case '%':
                break;

            /*
             * Conversion specifiers
             */

            case 'd': case 'i': case 'o': case 'u':
            case 'x': case 'a': case 'e': case 'f':
            case 'g':
                break;
            case 's':
                privatize_c_tx(va_arg(arg, void**), '\0',
                               PICOTM_TM_PRIVATIZE_LOAD);
                break;
            case 'n':
                privatize_tx(va_arg(arg, int*), sizeof(int),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;

            /*
             * Length modifiers
             */

            case 'l':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n': case 'a':
                    case 'A': case 'e': case 'E': case 'f':
                    case 'F': case 'g': case 'G': case 'c':
                    case '[': case 'm':
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 'j':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n':
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 'z':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n':
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 't':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n':
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 'L':
                switch (pos[2]) {
                    case 'a': case 'A': case 'e': case 'E':
                    case 'f': case 'F': case 'g': case 'G':
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            default:
                picotm_recover_from_errno(EINVAL);
                continue;
        }

        format = pos + 2;
    }
}

#if defined(PICOTM_LIBC_HAVE_VSNPRINTF) && PICOTM_LIBC_HAVE_VSNPRINTF
PICOTM_EXPORT
int
vsnprintf_tx(char* s, size_t n, const char* format, va_list ap)
{
    privatize_c_tx(s, n, PICOTM_TM_PRIVATIZE_STORE);
    privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);

    va_list ap_copy;
    va_copy(ap_copy, ap);
    privatize_printf_args(format, ap_copy);

    return vsnprintf_tm(s, n, format, ap);

}
#endif

static void
privatize_scanf_args(const char* format, va_list arg)
{
    while (format) {

        const char* pos = strchr(format, '%');
        if (!pos) {
            return;
        }

        switch (pos[1]) {

            case '%':
                break;

            /*
             * Conversion specifiers
             */

            case 'd':
                privatize_tx(va_arg(arg, int*), sizeof(int),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;
            case 'i':
                privatize_tx(va_arg(arg, int*), sizeof(int),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;
            case 'o':
                privatize_tx(va_arg(arg, unsigned*), sizeof(unsigned),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;
            case 'u':
                privatize_tx(va_arg(arg, unsigned*), sizeof(unsigned),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;
            case 'x':
                privatize_tx(va_arg(arg, unsigned*), sizeof(unsigned),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;
            case 'a': case 'e': case 'f':
                /* fall through */
            case 'g':
                privatize_tx(va_arg(arg, float*), sizeof(float),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;
            case 'p':
                privatize_tx(va_arg(arg, void**), sizeof(void*),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;
            case 'n':
                privatize_tx(va_arg(arg, int*), sizeof(int),
                             PICOTM_TM_PRIVATIZE_STORE);
                break;

            /*
             * Length modifiers
             */

            case 'l':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n':
                        privatize_tx(va_arg(arg, long*), sizeof(long),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    case 'a': case 'A': case 'e': case 'E':
                    case 'f': case 'F': case 'g': case 'G':
                        privatize_tx(va_arg(arg, double*), sizeof(double),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    case 'c': case 's': case '[':
                        privatize_tx(va_arg(arg, wchar_t*), sizeof(wchar_t),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    case 'm':
                        privatize_tx(va_arg(arg, wchar_t**), sizeof(wchar_t*),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 'j':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n':
                        privatize_tx(va_arg(arg, intmax_t*), sizeof(intmax_t),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 'z':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n':
                        privatize_tx(va_arg(arg, size_t*), sizeof(size_t),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 't':
                switch (pos[2]) {
                    case 'd': case 'i': case 'o': case 'u':
                    case 'x': case 'X': case 'n':
                        privatize_tx(va_arg(arg, ptrdiff_t*),
                                     sizeof(ptrdiff_t),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            case 'L':
                switch (pos[2]) {
                    case 'a': case 'A': case 'e': case 'E':
                    case 'f': case 'F': case 'g': case 'G':
                        privatize_tx(va_arg(arg, long double*),
                                     sizeof(long double),
                                     PICOTM_TM_PRIVATIZE_STORE);
                        break;
                    default:
                        picotm_recover_from_errno(EINVAL);
                        continue;
                }
                ++pos;
                break;

            default:
                picotm_recover_from_errno(EINVAL);
                continue;
        }

        format = pos + 2;
    }
}

#if defined(PICOTM_LIBC_HAVE_VSSCANF) && PICOTM_LIBC_HAVE_VSSCANF
PICOTM_EXPORT
int
vsscanf_tx(const char* s, const char* format, va_list arg)
{
    privatize_c_tx(s, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);

    va_list arg_copy;
    va_copy(arg_copy, arg);
    privatize_scanf_args(format, arg_copy);

    return vsscanf_tm(s, format, arg);
}
#endif
