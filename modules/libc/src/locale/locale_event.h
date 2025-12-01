/*
 * picotm - A system-level transaction manager
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

#include <locale.h>
#include "picotm/config/picotm-libc-config.h"

/**
 * \cond impl || libc_impl || libc_impl_locale
 * \ingroup libc_impl
 * \ingroup libc_impl_locale
 * \file
 * \endcond
 */

struct locale_tx;

/**
 * \brief Opcodes for locale events.
 */
enum locale_op {
    /** \brief Represents a duplocale() operation. */
    LOCALE_OP_DUPLOCALE,
    /** \brief Represents a freelocale() operation. */
    LOCALE_OP_FREELOCALE,
    /** \brief Represents a newlocale() operation. */
    LOCALE_OP_NEWLOCALE,
    /** \brief Represents a setlocale() operation. */
    LOCALE_OP_SETLOCALE,
    /** \brief Represents a uselocale() operation. */
    LOCALE_OP_USELOCALE,
    /** \brief The number of locale operations. */
    LAST_LOCALE_OP
};

/**
 * \brief Represents a locale event.
 */
struct locale_event {
    union {
#if defined(PICOTM_LIBC_HAVE_DUPLOCALE) && PICOTM_LIBC_HAVE_DUPLOCALE
        struct {
            locale_t result;
        } duplocale;
#endif
#if defined(PICOTM_LIBC_HAVE_FREELOCALE) && PICOTM_LIBC_HAVE_FREELOCALE
        struct {
            locale_t locobj;
        } freelocale;
#endif
#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && PICOTM_LIBC_HAVE_NEWLOCALE
        struct {
            locale_t result;
        } newlocale;
#endif
        struct {
            void* oldlocale;
            int category;
        } setlocale;
#if defined(PICOTM_LIBC_HAVE_USELOCALE) && PICOTM_LIBC_HAVE_USELOCALE
        struct {
            locale_t oldloc;
        } uselocale;
#endif
    } arg;
};
