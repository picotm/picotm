/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
