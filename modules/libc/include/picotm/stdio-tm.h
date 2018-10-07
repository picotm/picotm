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

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include <stdarg.h>
#include <stdio.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stdio.h>.
 */

#if defined(PICOTM_LIBC_HAVE_SNPRINTF) && PICOTM_LIBC_HAVE_SNPRINTF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [snprintf()][posix::snprintf].
 *
 * [posix::snprintf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/snprintf.html
 */
int
snprintf_tm(char* restrict s, size_t n, const char* restrict format, ...);
#endif

#if defined(PICOTM_LIBC_HAVE_SSCANF) && PICOTM_LIBC_HAVE_SSCANF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [sscanf()][posix::sscanf].
 *
 * [posix::sscanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sscanf.html
 */
int
sscanf_tm(const char* restrict s, const char* restrict format, ...);
#endif

#if defined(PICOTM_LIBC_HAVE_VSNPRINTF) && PICOTM_LIBC_HAVE_VSNPRINTF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [vsnprintf()][posix::vsnprintf].
 *
 * [posix::vsnprintf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/vsnprintf.html
 */
int
vsnprintf_tm(char* restrict s, size_t n, const char* restrict format,
             va_list ap);
#endif

#if defined(PICOTM_LIBC_HAVE_VSSCANF) && PICOTM_LIBC_HAVE_VSSCANF || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [vsscanf()][posix::vsscanf].
 *
 * [posix::vsscanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/vsscanf.html
 */
int
vsscanf_tm(const char* restrict s, const char* restrict format, va_list arg);
#endif

PICOTM_END_DECLS
