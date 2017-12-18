/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
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

static const char core_test_1_desc[] = "Test begin/commit/end combo.";

static void
core_test_1(unsigned int tid)
{
    picotm_begin
    picotm_commit
    picotm_end
}

/* Test 2
 */

static const char core_test_2_desc[] = "Test error recovery without restart.";

static void
core_test_2(unsigned int tid)
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

/* Test 3
 */

static const char core_test_3_desc[] = "Test error recovery with restart.";

static void
core_test_3(unsigned int tid)
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
    {core_test_3_desc, core_test_3, NULL, NULL}
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
