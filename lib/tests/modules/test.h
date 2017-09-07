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

#pragma once

enum boundary_type {
    BOUND_CYCLES = 0,
    BOUND_TIME
};

enum loop_mode {
    LOOP_INNER = 0,
    LOOP_OUTER
};

typedef void (* const pre_func)(unsigned long, enum loop_mode,
                                enum boundary_type, unsigned long long);
typedef void (* const post_func)(unsigned long, enum loop_mode,
                                 enum boundary_type, unsigned long long);
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
         unsigned long long bound);
