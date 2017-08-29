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

#include "tm_test.h"
#include <stdlib.h>
#include <picotm/picotm.h>
#include <picotm/picotm-tm.h>
#include "ptr.h"
#include "test.h"
#include "testhlp.h"

static unsigned long g_value;

static void
tm_test_1_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound,
              int (*logmsg)(const char*, ...))
{
    g_value = 0;
}

static void
tm_test_1(unsigned int tid)
{
    picotm_begin

        unsigned long value = load_ulong_tx(&g_value);
        value += 1;
        store_ulong_tx(&g_value, value);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static void
tm_test_1_post(unsigned long nthreads, enum loop_mode loop,
               enum boundary_type btype, unsigned long long bound,
               int (*logmsg)(const char*, ...))
{
    switch (btype) {
        case BOUND_CYCLES:
            if (g_value != (nthreads * bound)) {
                logmsg("post-condition failed: g_value != (nthreads * bound)\n");
                abort();
            }
            break;
        default:
            break;
    }
}

const struct test_func tm_test[] = {
    {"tm_test_1", tm_test_1, tm_test_1_pre, tm_test_1_post}
};

size_t
number_of_tm_tests()
{
    return arraylen(tm_test);
}
