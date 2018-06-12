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

#include "picotm/locale-tm.h"
#include "picotm/picotm-module.h"
#include "error/module.h"
#include "locale/module.h"

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && PICOTM_LIBC_HAVE_NEWLOCALE
PICOTM_EXPORT
locale_t
newlocale_tm(int category_mask, const char* locale, locale_t base)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        locale_t newloc = locale_module_newlocale(category_mask, locale,
                                                  base, &error);
        if (!picotm_error_is_set(&error)) {
            return newloc;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_SETLOCALE) && PICOTM_LIBC_HAVE_SETLOCALE
PICOTM_EXPORT
char*
setlocale_tm(int category, const char* locale)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        char* res = locale_module_setlocale(category, locale, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif
