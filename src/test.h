/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

enum boundary_type {
    BOUND_CYCLES = 0,
    BOUND_TIME
};

enum loop_mode {
    LOOP_INNER = 0,
    LOOP_OUTER
};

typedef void (* const pre_func)(void);
typedef void (* const post_func)(void);
typedef void (* const call_func)(unsigned int);

struct test_func
{
    const char* name;

    call_func call;
    pre_func  pre;
    post_func post;
};

long long
run_test(const struct test_func* test, unsigned long nthreads,
         enum loop_mode loop, enum boundary_type btype,
         unsigned long long bound, int (*logmsg)(const char*, ...));
