/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
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
#include "picotm/time.h"
#include <string.h>
#include "ptr.h"
#include "safeblk.h"
#include "safe_time.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

/*
 * Test strftime()
 */

static void
time_h_test_1(unsigned int tid)
{
    static const char format[] = "%D %T";

    time_t t = safe_time(&t);
    struct tm time;
    struct tm* timeptr = safe_gmtime_r(&t, &time);

    char buf1[256];
    memset(buf1, 0, sizeof(buf1));

    picotm_begin
        strftime_tx(buf1, sizeof(buf1) - 1, format, timeptr);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end

    char buf2[256];
    memset(buf2, 0, sizeof(buf2));

    strftime(buf2, sizeof(buf2) - 1, format, timeptr);

    if (strcmp(buf1, buf2)) {
        tap_error("time strings differ: '%s' '%s'\n", buf1, buf2);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_error_mark_as_non_recoverable(&error);
        picotm_recover_from_error(&error);
    }
}

/*
 * Test strptime()
 */

static void
time_h_test_2(unsigned int tid)
{
    static const char format[] = "%D %T";

    time_t t = safe_time(&t);
    struct tm time;
    struct tm* timeptr = safe_gmtime_r(&t, &time);

    char buf[256];
    memset(buf, 0, sizeof(buf));
    strftime(buf, sizeof(buf) - 1, format, timeptr);

    struct tm tm1;
    memset(&tm1, 0, sizeof(tm1));

    picotm_begin
        strptime_tx(buf, format, &tm1);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end

    struct tm tm2;
    memset(&tm2, 0, sizeof(tm2));
    strptime(buf, format, &tm2);

    if (memcmp(&tm1, &tm2, sizeof(tm1))) {
        tap_error("parsed times differ\n");
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_error_mark_as_non_recoverable(&error);
        picotm_recover_from_error(&error);
    }
}

static const struct test_func time_h_test[] = {
    {"Test strftime()", time_h_test_1, NULL, NULL},
    {"Test strptime()", time_h_test_2, NULL, NULL}
};

/*
 * Entry point
 */

#include "opts.h"
#include "pubapi.h"

int
main(int argc, char* argv[])
{
    return pubapi_main(argc, argv, PARSE_OPTS_STRING(),
                       time_h_test, arraylen(time_h_test));
}
