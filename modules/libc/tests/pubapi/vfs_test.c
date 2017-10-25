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
#include <picotm/picotm-error.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include <picotm/sched.h>
#include <picotm/stdio.h>
#include <picotm/unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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

static void
vfs_test_1(unsigned int tid)
{
    char format[PATH_MAX];
    test_dir_format_string(format, 1);

    char path[PATH_MAX];
    safe_snprintf(path, sizeof(path), format, tid);

	picotm_begin

        privatize_c_tx(format, '\0', PICOTM_TM_PRIVATIZE_LOAD);

        privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
        chdir_tx(path);

        sched_yield_tx();

        for (int i = 0; i < 5; ++i) {

            char cwdbuf[PATH_MAX];
            char* cwd = getcwd_tx(cwdbuf, sizeof(cwdbuf));

            /* FIXME: Issue #119: running non-tx code for testing
             * purposes **only** */
            unsigned long cwd_tid;
            int res = sscanf_tx(cwd, format, &cwd_tid);
            if (res < 1l) {
                tap_error("thread id did not match for CWD '%s'\n", cwd);
                abort_safe_block();
            }

            if (cwd_tid != tid) {
                tap_error("incorrect working directory\n");
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
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
        safe_snprintf(path, sizeof(path), format, tid);
        safe_mkdir(path, S_IRWXU);
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
        safe_snprintf(path, sizeof(path), format, tid);
        safe_rmdir(path);
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
