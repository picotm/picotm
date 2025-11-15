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

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include "picotm/picotm-tm.h"
#include <time.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <time.h>.
 */

#if defined(PICOTM_LIBC_HAVE_TYPE_STRUCT_ITIMERSPEC) && \
            PICOTM_LIBC_HAVE_TYPE_STRUCT_ITIMERSPEC || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(struct_itimerspec, struct itimerspec);
PICOTM_TM_STORE_TX(struct_itimerspec, struct itimerspec);
PICOTM_TM_PRIVATIZE_TX(struct_itimerspec, struct itimerspec);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_STRUCT_TIMESPEC) && \
            PICOTM_LIBC_HAVE_TYPE_STRUCT_TIMESPEC || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(struct_timespec, struct timespec);
PICOTM_TM_STORE_TX(struct_timespec, struct timespec);
PICOTM_TM_PRIVATIZE_TX(struct_timespec, struct timespec);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_STRUCT_TM) && \
            PICOTM_LIBC_HAVE_TYPE_STRUCT_TM || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(struct_tm, struct tm);
PICOTM_TM_STORE_TX(struct_tm, struct tm);
PICOTM_TM_PRIVATIZE_TX(struct_tm, struct tm);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_STRFTIME) && \
            PICOTM_LIBC_HAVE_STRFTIME || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [strftime()][posix::strftime].
 *
 * [posix::strftime]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/strftime.html
 */
size_t
strftime_tx(char* restrict s, size_t maxsize, const char* restrict format,
            const struct tm* restrict timeptr);
#endif

#if defined(PICOTM_LIBC_HAVE_STRFTIME_L) && \
            PICOTM_LIBC_HAVE_STRFTIME_L || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [strftime_l()][posix::strftime_l].
 *
 * [posix::strftime_l]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/strftime_l.html
 */
size_t
strftime_l_tx(char* restrict s, size_t maxsize, const char* restrict format,
              const struct tm* restrict timeptr, locale_t locale);
#endif

#if defined(PICOTM_LIBC_HAVE_STRPTIME) && \
            PICOTM_LIBC_HAVE_STRPTIME || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [strptime()][posix::strptime].
 *
 * [posix::strptime]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/strptime.html
 */
char*
strptime_tx(const char* restrict buf, const char* restrict format,
            struct tm* restrict tm);
#endif

#if defined(PICOTM_LIBC_HAVE_STRPTIME) && \
            PICOTM_LIBC_HAVE_STRPTIME || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [strptime()][posix::strptime].
 *
 * [posix::strptime]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/strptime.html
 */
char*
strptime(const char* restrict buf, const char* restrict format,
         struct tm* restrict tm);
#endif

PICOTM_END_DECLS
