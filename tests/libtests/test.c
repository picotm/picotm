/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
 * Copyright (c) 2019   Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "test.h"
#include <assert.h>
#include "safeblk.h"
#include "taputils.h"
#include "tap.h"

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

void
run_tests(const struct test_func* beg, const struct test_func* end,
          unsigned long nthreads,
          enum loop_mode loop, enum boundary_type btype,
          unsigned long long limit)
{
    tap_plan(end - beg);

    volatile const struct test_func* pos = beg;

    for (unsigned long testnum = 1; pos < end; ++testnum, ++pos) {

        int test_aborted = 0;

        begin_safe_block(test_aborted)

            run_test((const struct test_func*)pos, nthreads,
                     loop, btype, limit);

        end_safe_block

        if (test_aborted) {
            tap_not_ok(testnum, beg->name);
        } else {
            tap_ok(testnum, beg->name);
        }
    }
}
