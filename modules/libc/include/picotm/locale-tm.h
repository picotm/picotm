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
#include <locale.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <locale.h>.
 */

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && \
            PICOTM_LIBC_HAVE_NEWLOCALE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of newlocale_tx() that operates on transactional memory.
 */
locale_t
newlocale_tm(int category_mask, const char* locale, locale_t base);
#endif

#if defined(PICOTM_LIBC_HAVE_SETLOCALE) && \
            PICOTM_LIBC_HAVE_SETLOCALE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * Variant of setlocale_tx() that operates on transactional memory.
 */
char*
setlocale_tm(int category, const char* locale);
#endif

PICOTM_END_DECLS
