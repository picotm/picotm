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

#include "picotm/locale.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include "error/module.h"
#include "locale/module.h"
#include "picotm/locale-tm.h"

#if defined(PICOTM_LIBC_HAVE_DUPLOCALE) && PICOTM_LIBC_HAVE_DUPLOCALE
PICOTM_EXPORT
locale_t
duplocale_tx(locale_t locobj)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        error_module_save_errno(&error);
        if (!picotm_error_is_set(&error)) {
            break;
        }
        picotm_recover_from_error(&error);
    } while (true);

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        locale_t newloc = locale_module_duplocale(locobj, &error);
        if (!picotm_error_is_set(&error)) {
            return newloc;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_FREELOCALE) && PICOTM_LIBC_HAVE_FREELOCALE
PICOTM_EXPORT
void
freelocale_tx(locale_t locobj)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        locale_module_freelocale(locobj, &error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_LOCALECONV) && PICOTM_LIBC_HAVE_LOCALECONV
PICOTM_EXPORT
struct lconv*
localeconv_tx()
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct lconv* res = locale_module_localeconv(&error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && PICOTM_LIBC_HAVE_NEWLOCALE
PICOTM_EXPORT
locale_t
newlocale_tx(int category_mask, const char* locale, locale_t base)
{
    privatize_c_tx(locale, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return newlocale_tm(category_mask, locale, base);
}
#endif

#if defined(PICOTM_LIBC_HAVE_SETLOCALE) && PICOTM_LIBC_HAVE_SETLOCALE
PICOTM_EXPORT
char*
setlocale_tx(int category, const char* locale)
{
    if (locale) {
        /* locale is be NULL when caller queries current setting */
        privatize_c_tx(locale, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    }
    return setlocale_tm(category, locale);
}
#endif

#if defined(PICOTM_LIBC_HAVE_USELOCALE) && PICOTM_LIBC_HAVE_USELOCALE
PICOTM_EXPORT
locale_t
uselocale_tx(locale_t newloc)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        error_module_save_errno(&error);
        if (!picotm_error_is_set(&error)) {
            break;
        }
        picotm_recover_from_error(&error);
    } while (true);

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        locale_t oldloc = locale_module_uselocale(newloc, &error);
        if (!picotm_error_is_set(&error)) {
            return oldloc;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif
