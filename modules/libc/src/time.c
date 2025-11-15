/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
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

#include "picotm/time.h"

#if defined(PICOTM_LIBC_HAVE_STRFTIME) && \
            PICOTM_LIBC_HAVE_STRFTIME
PICOTM_EXPORT
size_t
strftime_tx(char* restrict s, size_t maxsize, const char* restrict format,
            const struct tm* restrict timeptr)
{
    privatize_tx(s, maxsize, PICOTM_TM_PRIVATIZE_STORE);
    privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_struct_tm_tx(timeptr, PICOTM_TM_PRIVATIZE_LOAD);
    return strftime(s, maxsize, format, timeptr);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRFTIME_L) && \
            PICOTM_LIBC_HAVE_STRFTIME_L
PICOTM_EXPORT
size_t
strftime_l_tx(char* restrict s, size_t maxsize, const char* restrict format,
              const struct tm* restrict timeptr, locale_t locale)
{
    privatize_tx(s, maxsize, PICOTM_TM_PRIVATIZE_STORE);
    privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_struct_tm_tx(timeptr, PICOTM_TM_PRIVATIZE_LOAD);
    return strftime_l(s, maxsize, format, timeptr, locale);
}
#endif

#if defined(PICOTM_LIBC_HAVE_STRPTIME) && \
            PICOTM_LIBC_HAVE_STRPTIME
PICOTM_EXPORT
char*
strptime_tx(const char* restrict buf, const char* restrict format,
            struct tm* restrict tm)
{
    privatize_c_tx(buf, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_struct_tm_tx(tm, PICOTM_TM_PRIVATIZE_STORE);
    return strptime(buf, format, tm);
}
#endif
