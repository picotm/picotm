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
#include <picotm/picotm.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include <stdlib.h>
#include <string.h>
#include "ptr.h"
#include "safeblk.h"
#include "safe_stdio.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

#define STRSIZE 128

static unsigned long g_value;
static char          g_string[STRSIZE];

/**
 * Load a shared value.
 */
static void
tm_test_1(unsigned int tid)
{
    picotm_begin

        unsigned long value = load_ulong_tx(&g_value);
        if (!(value == 0)) {
            tap_error("condition failed: value == 0");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static void
tm_test_1_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound)
{
    g_value = 0;
}

static void
tm_test_1_post(unsigned long nthreads, enum loop_mode loop,
               enum boundary_type btype, unsigned long long bound)
{
    if (!(g_value == 0)) {
        tap_error("post-condition failed: g_value == 0");
        abort_safe_block();
    }
}

/**
 * Store to a shared value.
 */
static void
tm_test_2(unsigned int tid)
{
    picotm_begin

        store_ulong_tx(&g_value, tid);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static void
tm_test_2_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound)
{
    g_value = nthreads;
}

static void
tm_test_2_post(unsigned long nthreads, enum loop_mode loop,
               enum boundary_type btype, unsigned long long bound)
{
    if (!(g_value < nthreads)) {
        tap_error("post-condition failed: g_value < nthreads");
        abort_safe_block();
    }
}

/**
 * Load, add and store a shared value.
 */
static void
tm_test_3(unsigned int tid)
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
tm_test_3_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound)
{
    g_value = 0;
}

static void
tm_test_3_post(unsigned long nthreads, enum loop_mode loop,
               enum boundary_type btype, unsigned long long bound)
{
    switch (btype) {
        case CYCLE_BOUND:
            if (!(g_value == (nthreads * bound))) {
                tap_error("post-condition failed: g_value == (nthreads * bound)");
                abort_safe_block();
            }
            break;
        case TIME_BOUND:
            break;
    }
}

/**
 * Store, load and compare a shared value.
 */
static void
tm_test_4(unsigned int tid)
{
    picotm_begin

        store_ulong_tx(&g_value, tid);
        unsigned long value = load_ulong_tx(&g_value);

        if (!(value == tid)) {
            tap_error("condition failed: value == tid");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static void
tm_test_4_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound)
{
    g_value = nthreads;
}

/**
 * Privatize, store, load and compare a shared value.
 */
static void
tm_test_5(unsigned int tid)
{
    picotm_begin

        privatize_ulong_tx(&g_value, PICOTM_TM_PRIVATIZE_LOADSTORE);

        g_value = tid;
        unsigned long value = g_value;

        if (!(value == tid)) {
            tap_error("condition failed: value == tid");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static void
tm_test_5_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound)
{
    g_value = nthreads;
}

/**
 * Privatize and memcpy a shared string.
 */
static void
tm_test_6(unsigned int tid)
{
    static const char format[] = "tid %lu";

    char istr[STRSIZE];
    char ostr[STRSIZE];
    safe_snprintf(istr, sizeof(istr), format, tid);

    picotm_begin

        privatize_c_tx(istr, '\0', PICOTM_TM_PRIVATIZE_LOAD);

        privatize_tx(ostr, sizeof(ostr), PICOTM_TM_PRIVATIZE_STORE);
        privatize_tx(g_string, sizeof(g_string), PICOTM_TM_PRIVATIZE_LOADSTORE);

        memcpy(g_string, istr, sizeof(g_string));
        memcpy(ostr, g_string, sizeof(ostr));

        g_value = tid;
        unsigned long value = g_value;

        if (!(value == tid)) {
            tap_error("condition failed: value == tid");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    unsigned long value;
    int len = safe_sscanf(ostr, format, &value);

    if (!(len == 1)) {
        tap_error("condition failed: len == 1");
        abort_safe_block();
    }

    if (!(value == tid)) {
        tap_error("condition failed: value == tid");
        abort_safe_block();
    }
}

const struct test_func tm_test[] = {
    {"tm_test_1", tm_test_1, tm_test_1_pre, tm_test_1_post},
    {"tm_test_2", tm_test_2, tm_test_2_pre, tm_test_2_post},
    {"tm_test_3", tm_test_3, tm_test_3_pre, tm_test_3_post},
    {"tm_test_4", tm_test_4, tm_test_4_pre, NULL},
    {"tm_test_5", tm_test_5, tm_test_5_pre, NULL},
    {"tm_test_6", tm_test_6, NULL, NULL}
};

size_t
number_of_tm_tests()
{
    return arraylen(tm_test);
}
