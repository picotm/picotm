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

#include <locale.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \cond impl || libc_impl || libc_impl_locale
 * \ingroup libc_impl
 * \ingroup libc_impl_locale
 * \file
 * \endcond
 */

struct picotm_error;

#if defined(PICOTM_LIBC_HAVE_DUPLOCALE) && PICOTM_LIBC_HAVE_DUPLOCALE
locale_t
locale_module_duplocale(locale_t locobj, struct picotm_error* error);
#endif

#if defined(PICOTM_LIBC_HAVE_FREELOCALE) && PICOTM_LIBC_HAVE_FREELOCALE
void
locale_module_freelocale(locale_t locobj, struct picotm_error* error);
#endif

struct lconv*
locale_module_localeconv(struct picotm_error* error);

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && PICOTM_LIBC_HAVE_NEWLOCALE
locale_t
locale_module_newlocale(int category_mask, const char* locale,
                        locale_t base, struct picotm_error* error);
#endif

char*
locale_module_setlocale(int category, const char* locale,
                        struct picotm_error* error);

#if defined(PICOTM_LIBC_HAVE_USELOCALE) && PICOTM_LIBC_HAVE_USELOCALE
locale_t
locale_module_uselocale(locale_t newloc, struct picotm_error* error);
#endif
