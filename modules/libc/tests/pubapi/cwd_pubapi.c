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

#include <limits.h>
#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include <picotm/sched.h>
#include <picotm/stdio-tm.h>
#include <picotm/unistd.h>
#include <picotm/unistd-tm.h>
#include <string.h>
#include "ptr.h"
#include "safeblk.h"
#include "safe_stdio.h"
#include "safe_sys_stat.h"
#include "safe_unistd.h"
#include "taputils.h"
#include "tempfile.h"
#include "test.h"
#include "testhlp.h"

void
test_dir_format_string(char format[PATH_MAX], unsigned long test)
{
    safe_snprintf(format, PATH_MAX, "%s/vfs_test_%lu-%%lu.test", temp_path(),
                  test);
}

/*
 * Test getcwd()
 */

static void
cwd_test_1(unsigned int tid)
{
    const char* cwd = safe_getcwd(NULL, 0);

	picotm_begin

        privatize_c_tx(cwd, '\0', PICOTM_TM_PRIVATIZE_LOAD);

        char* cwd_tx = getcwd_tx(NULL, 0);

        if (strcmp(cwd, cwd_tx)) {
            tap_error("working directories did not match:"
                      " expected '%s', got '%s'\n", cwd, cwd_tx);
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        delay_transaction_tx(tid);

    picotm_commit

        abort_transaction_on_error(__func__);

	picotm_end
}

/*
 * chdir()
 */

static void
cwd_test_2(unsigned int tid)
{
    char format[PATH_MAX];
    test_dir_format_string(format, 1);

    char path[PATH_MAX];
    safe_snprintf(path, sizeof(path), format, tid);

	picotm_begin

        privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);

        privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
        chdir_tx(path);

        delay_transaction_tx(tid);

        for (int i = 0; i < 5; ++i) {

            char cwdbuf[PATH_MAX];
            char* cwd = getcwd_tm(cwdbuf, sizeof(cwdbuf));

            unsigned long cwd_tid;
            int res = sscanf_tm(cwd, format, &cwd_tid);
            if (res < 1l) {
                tap_error("thread id did not match for CWD '%s'\n", cwd);
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
            }

            if (cwd_tid != tid) {
                tap_error("incorrect working directory\n");
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
            }

            delay_transaction_tx(tid);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

	picotm_end
}

static void
cwd_test_2_pre(unsigned long nthreads, enum loop_mode loop,
               enum boundary_type btype, unsigned long long bound)
{
    char format[PATH_MAX];
    test_dir_format_string(format, 1);

    for (unsigned long tid = 0; tid < nthreads; ++tid) {

        char path[PATH_MAX];
        safe_snprintf(path, sizeof(path), format, tid);
        safe_mkdir(path, S_IRWXU);
    }
}

static void
cwd_test_2_post(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound)
{
    char format[PATH_MAX];
    test_dir_format_string(format, 1);

    for (unsigned long tid = 0; tid < nthreads; ++tid) {

        char path[PATH_MAX];
        safe_snprintf(path, sizeof(path), format, tid);
        safe_rmdir(path);
    }
}

static const struct test_func cwd_test[] = {
    {"cwd_test_1", cwd_test_1, NULL,           NULL},
    {"cwd_test_2", cwd_test_2, cwd_test_2_pre, cwd_test_2_post},
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
                       cwd_test, arraylen(cwd_test));
}
