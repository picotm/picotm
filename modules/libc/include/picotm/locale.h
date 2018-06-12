/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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
#include <locale.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <locale.h>.
 */

#if defined(PICOTM_LIBC_HAVE_TYPE_LOCALE_T) && \
            PICOTM_LIBC_HAVE_TYPE_LOCALE_T || \
    defined(__PICOTM_DOXYGEN)
#if !defined(__PICOTM_LOAD_LOCALE_T_TX) || \
            !__PICOTM_LOAD_LOCALE_T_TX
#undef __PICOTM_LOAD_LOCALE_T_TX
#define __PICOTM_LOAD_LOCALE_T_TX   (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(locale_t, locale_t);
/** \} */
#endif
#if !defined(__PICOTM_STORE_LOCALE_T_TX) || \
            !__PICOTM_STORE_LOCALE_T_TX
#undef __PICOTM_STORE_LOCALE_T_TX
#define __PICOTM_STORE_LOCALE_T_TX  (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_STORE_TX(locale_t, locale_t);
/** \} */
#endif
#if !defined(__PICOTM_PRIVATIZE_LOCALE_T_TX) || \
            !__PICOTM_PRIVATIZE_LOCALE_T_TX
#undef __PICOTM_PRIVATIZE_LOCALE_T_TX
#define __PICOTM_PRIVATIZE_LOCALE_T_TX  (1)
PICOTM_TM_PRIVATIZE_TX(locale_t, locale_t);
/** \} */
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_STRUCT_LCONV) && \
            PICOTM_LIBC_HAVE_TYPE_STRUCT_LCONV || \
    defined(__PICOTM_DOXYGEN)
#if !defined(__PICOTM_LOAD_STRUCT_LCONV_TX) || \
            !__PICOTM_LOAD_STRUCT_LCONV_TX
#undef __PICOTM_LOAD_STRUCT_LCONV_TX
#define __PICOTM_LOAD_STRUCT_LCONV_TX   (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(struct_lconv, struct lconv);
/** \} */
#endif
#if !defined(__PICOTM_STORE_STRUCT_LCONV_TX) || \
            !__PICOTM_STORE_STRUCT_LCONV_TX
#undef __PICOTM_STORE_STRUCT_LCONV_TX
#define __PICOTM_STORE_STRUCT_LCONV_TX  (1)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_STORE_TX(struct_lconv, struct lconv);
/** \} */
#endif
#if !defined(__PICOTM_PRIVATIZE_STRUCT_LCONV_TX) || \
            !__PICOTM_PRIVATIZE_STRUCT_LCONV_TX
#undef __PICOTM_PRIVATIZE_STRUCT_LCONV_TX
#define __PICOTM_PRIVATIZE_STRUCT_LCONV_TX  (1)
PICOTM_TM_PRIVATIZE_TX(struct_lconv, struct lconv);
/** \} */
#endif
#endif

#if defined(PICOTM_LIBC_HAVE_DUPLOCALE) && \
            PICOTM_LIBC_HAVE_DUPLOCALE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [duplocale()][posix::duplocale].
 *
 * [posix::duplocale]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/duplocale.html
 */
locale_t
duplocale_tx(locale_t locobj);
#endif

#if defined(PICOTM_LIBC_HAVE_FREELOCALE) && \
            PICOTM_LIBC_HAVE_FREELOCALE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [freelocale()][posix::freelocale].
 *
 * [posix::freelocale]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/freelocale.html
 */
void
freelocale_tx(locale_t locobj);
#endif

#if defined(PICOTM_LIBC_HAVE_LOCALECONV) && \
            PICOTM_LIBC_HAVE_LOCALECONV || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [localeconv()][posix::localeconv].
 *
 * [posix::localeconv]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/localeconv.html
 */
struct lconv*
localeconv_tx(void);
#endif

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && \
            PICOTM_LIBC_HAVE_NEWLOCALE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [newlocale()][posix::newlocale].
 *
 * [posix::newlocale]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/newlocale.html
 */
locale_t
newlocale_tx(int category_mask, const char* locale, locale_t base);
#endif

#if defined(PICOTM_LIBC_HAVE_SETLOCALE) && \
            PICOTM_LIBC_HAVE_SETLOCALE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [setlocale()][posix::setlocale].
 *
 * [posix::setlocale]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/setlocale.html
 */
char*
setlocale_tx(int category, const char* locale);
#endif

#if defined(PICOTM_LIBC_HAVE_USELOCALE) && \
            PICOTM_LIBC_HAVE_USELOCALE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [uselocale()][posix::uselocale].
 *
 * [posix::uselocale]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/uselocale.html
 */
locale_t
uselocale_tx(locale_t newloc);
#endif

PICOTM_END_DECLS
