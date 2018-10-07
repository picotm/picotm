/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "picotm/picotm.h"
#include "picotm/picotm-module.h"
#include "picotm/locale.h"
#include "picotm/stdlib.h"
#include "picotm/string.h"
#include "picotm/string-tm.h"
#include <string.h>
#include "ptr.h"
#include "safeblk.h"
#include "safe_time.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

/* For testing, pick a locale that is probably available on the system. */
static const char test_locale[] = "en_US.UTF-8";

/*
 * Test setlocale()
 */

static void
locale_h_test_1(unsigned int tid)
{
    picotm_begin
        setlocale_tx(LC_ALL, NULL); /* query locale */
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

static void
locale_h_test_2(unsigned int tid)
{
    picotm_begin
        char* locale1 = strdup_tx(setlocale_tx(LC_ALL, NULL));
        char* locale2 = strdup_tx(setlocale_tx(LC_ALL, test_locale));

        /* check for correctly updated locale */
        int cmp = strcmp_tm(locale2, test_locale);
        if (cmp) {
          tap_error("locale2 differs from test locale, cmp=%d\n", cmp);
          struct picotm_error error = PICOTM_ERROR_INITIALIZER;
          picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
          picotm_error_mark_as_non_recoverable(&error);
          picotm_recover_from_error(&error);
        }

        /* check original locale against updated one */
        int cmp1 = strcmp_tm(locale1, locale2);
        int cmp2 = strcmp_tm(locale1, test_locale);
        if (cmp1 != cmp2) {
          tap_error("locale1 compares different to locale2 and test locale, "
                    "%d != %d\n", cmp1, cmp2);
          struct picotm_error error = PICOTM_ERROR_INITIALIZER;
          picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
          picotm_error_mark_as_non_recoverable(&error);
          picotm_recover_from_error(&error);
        }

        free_tx(locale2);
        free_tx(locale1);

    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test localeconv()
 */

static void
locale_h_test_3(unsigned int tid)
{
    picotm_begin
        struct lconv *lc = localeconv_tx();

        if (!lc) {
            tap_error("condition failed: no lc");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test POSIX locale interface
 */

static void
locale_h_test_4(unsigned int tid)
{
    picotm_begin
        locale_t locobj = newlocale_tx(LC_ALL, test_locale, (locale_t)0);
        freelocale_tx(locobj);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

static void
locale_h_test_5(unsigned int tid)
{
    picotm_begin
        locale_t locobj1 = newlocale_tx(LC_ALL, test_locale, (locale_t)0);
        locale_t locobj2 = duplocale_tx(locobj1);
        freelocale_tx(locobj1);
        freelocale_tx(locobj2);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

static void
locale_h_test_6(unsigned int tid)
{
    locale_t locobj = (locale_t)0;

    picotm_begin
        locale_t tx_locobj = newlocale_tx(LC_ALL, test_locale, (locale_t)0);
        uselocale_tx(tx_locobj);
        store_locale_t_tx(&locobj, tx_locobj);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end

    if (locobj == (locale_t)0) {
        tap_error("lobobj was not updated");
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_error_mark_as_non_recoverable(&error);
        picotm_recover_from_error(&error);
    }

    const locale_t cur_locobj = uselocale((locale_t)0);

    if (cur_locobj != locobj) {
        tap_error("thread-local locale was not set: %p != %p",
                  cur_locobj, locobj);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_error_mark_as_non_recoverable(&error);
        picotm_recover_from_error(&error);
    }

    picotm_begin
        locale_t tx_locobj = load_locale_t_tx(&locobj);
        freelocale_tx(tx_locobj);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Entry point
 */

static const struct test_func locale_h_test[] = {
    {"Test setlocale(NULL)",               locale_h_test_1, NULL, NULL},
    {"Test setlocale(<test locale>)",      locale_h_test_2, NULL, NULL},
    {"Test localeconv()",                  locale_h_test_3, NULL, NULL},
    {"Create and free locale",             locale_h_test_4, NULL, NULL},
    {"Create, duplicate and free locale",  locale_h_test_5, NULL, NULL},
    {"Create and use thread-local locale", locale_h_test_6, NULL, NULL}
};

#include "opts.h"
#include "pubapi.h"

int
main(int argc, char* argv[])
{
    return pubapi_main(argc, argv, PARSE_OPTS_STRING(),
                       locale_h_test, arraylen(locale_h_test));
}
