/*
 * picotm - A system-level transaction manager
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
#include <string.h>
#include "ptr.h"
#include "safeblk.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

/*
 * Test core interface and functionality
 */

/* Test 1
 */

static const char core_test_1_desc[] = "Test clean-up without transaction.";

static void
core_test_1(unsigned int tid)
{
    /* The caller runs the thread-local clean-up code in picotm_release(). */
    return;
}

/* Test 2
 */

static const char core_test_2_desc[] = "Test begin/commit/end combo.";

static void
core_test_2(unsigned int tid)
{
    picotm_begin
    picotm_commit
    picotm_end
}

/* Test 3
 */

static const char core_test_3_desc[] = "Test error recovery without restart.";

static void
core_test_3(unsigned int tid)
{
    picotm_safe bool performed_error_recovery = false;

    picotm_begin

        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);

    picotm_commit

        performed_error_recovery = true;
        /* leave error recovery without restarting TX */

    picotm_end

    if (!performed_error_recovery) {
        tap_error("Transaction did not perform error recovery.\n");
        abort_safe_block();
    }
}

/* Test 4
 */

static const char core_test_4_desc[] = "Test error recovery with restart.";

static void
core_test_4(unsigned int tid)
{
    picotm_safe bool performed_error_recovery = false;
    picotm_safe bool performed_restart = false;

    picotm_begin

        if (!performed_error_recovery) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_recover_from_error(&error);
        } else {
            performed_restart = true;
        }

    picotm_commit

        performed_error_recovery = true;

        if (!performed_restart) {
            picotm_restart();
        }

    picotm_end

    if (!performed_error_recovery) {
        tap_error("Transaction did not perform error recovery.\n");
        abort_safe_block();
    }
    if (!performed_restart) {
        tap_error("Transaction did not restart after error recovery.\n");
        abort_safe_block();
    }
}

static const struct test_func core_test[] = {
    {core_test_1_desc, core_test_1, NULL, NULL},
    {core_test_2_desc, core_test_2, NULL, NULL},
    {core_test_3_desc, core_test_3, NULL, NULL},
    {core_test_4_desc, core_test_4, NULL, NULL}
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
                       core_test, arraylen(core_test));
}
