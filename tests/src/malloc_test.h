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

#include "test.h"

void malloc_test_1(unsigned int tid);
void malloc_test_2(unsigned int tid);
void malloc_test_3(unsigned int tid);
void malloc_test_4(unsigned int tid);
void malloc_test_5(unsigned int tid);
void malloc_test_6(unsigned int tid);

void malloc_test_7_pre(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));
void malloc_test_7(unsigned int tid);

void malloc_test_8_pre(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));
void malloc_test_8(unsigned int tid);

void malloc_test_9_pre(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));
void malloc_test_9(unsigned int tid);
