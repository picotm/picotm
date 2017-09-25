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

#include "test.h"
#include <assert.h>
/*
#include <stdlib.h>
#include <picotm/picotm.h>*/
#include "taputils.h"
/*
#include "test_state.h"
#include "safe_pthread.h"
#include "safe_stdlib.h"
#include "safe_sys_time.h"*/

static void
call(unsigned int tid, void* data)
{
    struct test_func* test = data;
    assert(test);

    test->call(tid);
}

void
run_test(const struct test_func* test, unsigned long nthreads,
         enum loop_mode loop, enum boundary_type btype,
         unsigned long long limit)
{
    assert(test);

    tap_info("Running test %s...", test->name);

    if (test->pre) {
        test->pre(nthreads, loop, btype, limit);
    }

    spawn_threads(nthreads, call, (void*)test, loop, btype, limit);

    if (test->post) {
        test->post(nthreads, loop, btype, limit);
    }
}