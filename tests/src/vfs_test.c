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
#include "test.h"
#include "testhlp.h"

static void
vfs_test_1(unsigned int tid)
{
    char path[64];
    int res = snprintf(path, sizeof(path), "/tmp/vfs_test_1-%u.test", tid);
    if (res < 0) {
        perror("snprintf");
        abort();
    }

	picotm_begin

        privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
        chdir_tx(path);

        sched_yield_tx();

        for (int i = 0; i < 5; ++i) {

            char cwdbuf[PATH_MAX];
            char* cwd = getcwd_tx(cwdbuf, sizeof(cwdbuf));

            /* running non-tx code for testing purposes **only** */
            unsigned int cwd_tid;
            int res = sscanf(cwd, "/tmp/vfs_test_1-%u.test", &cwd_tid);
            if (res < 0) {
                perror("sscanf");
                abort();
            } if ((size_t)res < 1) {
                fprintf(stderr, "thread id did not match for CWD '%s'\n", cwd);
                abort();
            }

            if (cwd_tid != tid) {
                fprintf(stderr, "incorrect working directory\n");
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
               enum boundary_type btype, unsigned long long bound,
               int (*logmsg)(const char*, ...))
{
    for (unsigned long tid = 0; tid < nthreads; ++tid) {

        char path[64];
        int res = snprintf(path, sizeof(path), "/tmp/vfs_test_1-%lu.test", tid);
        if (res < 0) {
            perror("snprintf");
            abort();
        }

        res = mkdir(path, S_IRWXU);
        if (res < 0) {
            perror("mkdir");
            abort();
        }
    }
}

static void
vfs_test_1_post(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    for (unsigned long tid = 0; tid < nthreads; ++tid) {

        char path[64];
        int res = snprintf(path, sizeof(path), "/tmp/vfs_test_1-%lu.test", tid);
        if (res < 0) {
            perror("snprintf");
            abort();
        }

        res = rmdir(path);
        if (res < 0) {
            perror("rmdir");
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
