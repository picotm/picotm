/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#include "vfs_test.h"
#include <errno.h>
#include <limits.h>
#include <picotm/picotm.h>
#include <picotm/picotm-tm.h>
#include <picotm/sched.h>
#include <picotm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ptr.h"
#include "taputils.h"
#include "tempfile.h"
#include "test.h"
#include "testhlp.h"

void
test_dir_format_string(char format[PATH_MAX], unsigned long test)
{
    int res = snprintf(format, PATH_MAX, "%s/vfs_test_%lu-%%lu.test", temp_path(), test);
    if (res < 0) {
        tap_error_errno("snprintf()", errno);
        abort();
    } else if (res >= PATH_MAX) {
        tap_error("snprintf() output truncated\n");
        abort();
    }
}

static void
vfs_test_1(unsigned int tid)
{
    char format[PATH_MAX];
    test_dir_format_string(format, 1);

    char path[PATH_MAX];
    int res = snprintf(path, sizeof(path), format, tid);
    if (res < 0) {
        tap_error_errno("snprintf()", errno);
        abort();
    }

	picotm_begin

        privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);

        privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
        chdir_tx(path);

        sched_yield_tx();

        for (int i = 0; i < 5; ++i) {

            char cwdbuf[PATH_MAX];
            char* cwd = getcwd_tx(cwdbuf, sizeof(cwdbuf));

            /* running non-tx code for testing purposes **only** */
            unsigned long cwd_tid;
            int res = sscanf(cwd, format, &cwd_tid);
            if (res < 0) {
                tap_error_errno("sscanf()", errno);
                abort();
            } if ((size_t)res < 1) {
                tap_error("thread id did not match for CWD '%s'\n", cwd);
                abort();
            }

            if (cwd_tid != tid) {
                tap_error("incorrect working directory\n");
                abort();
            }

            sleep_tx(1);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

	picotm_end
}

static void
vfs_test_1_pre(unsigned long nthreads, enum loop_mode loop,
               enum boundary_type btype, unsigned long long bound)
{
    char format[PATH_MAX];
    test_dir_format_string(format, 1);

    for (unsigned long tid = 0; tid < nthreads; ++tid) {

        char path[PATH_MAX];
        int res = snprintf(path, sizeof(path), format, tid);
        if (res < 0) {
            tap_error_errno("snprintf()", errno);
            abort();
        }

        res = mkdir(path, S_IRWXU);
        if (res < 0) {
            tap_error_errno("mkdir()", errno);
            abort();
        }
    }
}

static void
vfs_test_1_post(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound)
{
    char format[PATH_MAX];
    test_dir_format_string(format, 1);

    for (unsigned long tid = 0; tid < nthreads; ++tid) {

        char path[PATH_MAX];
        int res = snprintf(path, sizeof(path), format, tid);
        if (res < 0) {
            tap_error_errno("snprintf()", errno);
            abort();
        }

        res = rmdir(path);
        if (res < 0) {
            tap_error_errno("rmdir()", errno);
            abort();
        }
    }
}

const struct test_func vfs_test[] = {
    {"vfs_test_1", vfs_test_1, vfs_test_1_pre, vfs_test_1_post},
};

size_t
number_of_vfs_tests()
{
    return arraylen(vfs_test);
}
