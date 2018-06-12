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

#include "picotm/locale.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include "locale/module.h"
#include "picotm/locale-tm.h"

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
