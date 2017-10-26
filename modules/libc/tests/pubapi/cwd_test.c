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

#include "cwd_test.h"
#include <picotm/picotm.h>
#include <picotm/picotm-tm.h>
#include <picotm/unistd.h>
#include <string.h>
#include "delay.h"
#include "ptr.h"
#include "safeblk.h"
#include "safe_unistd.h"
#include "taputils.h"
#include "testhlp.h"

/**
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
            abort_safe_block();
        }

        delay_transaction(tid);

    picotm_commit

        abort_transaction_on_error(__func__);

	picotm_end
}

const struct test_func cwd_test[] = {
    {"cwd_test_1", cwd_test_1, NULL, NULL},
};

size_t
number_of_cwd_tests()
{
    return arraylen(cwd_test);
}
