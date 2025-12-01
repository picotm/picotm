/*
 * picotm - A system-level transaction manager
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

#pragma once

#include "thread.h"

typedef void (* const pre_func)(unsigned long, enum loop_mode,
                                enum boundary_type, unsigned long long);
typedef void (* const post_func)(unsigned long, enum loop_mode,
                                 enum boundary_type, unsigned long long);
typedef void (* const call_func)(unsigned int);

struct test_func {

    const char* name;

    call_func call;
    pre_func  pre;
    post_func post;
};

void
run_test(const struct test_func* test, unsigned long nthreads,
         enum loop_mode loop, enum boundary_type btype,
         unsigned long long limit);

void
run_tests(const struct test_func* beg, const struct test_func* end,
          unsigned long nthreads, enum loop_mode loop,
          enum boundary_type btype, unsigned long long limit);
