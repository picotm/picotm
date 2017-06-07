/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tm_test.h"
#include <stdlib.h>
#include <picotm/picotm.h>
#include <picotm/picotm-tm.h>
#include "testhlp.h"

static unsigned long g_value;

void
tm_test_1_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound,
              int (*logmsg)(const char*, ...))
{
    g_value = 0;
}

void
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

void
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
