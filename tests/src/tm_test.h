/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "test.h"

void
tm_test_1_pre(unsigned long nthreads, enum loop_mode loop,
              enum boundary_type btype, unsigned long long bound,
              int (*logmsg)(const char*, ...));
void
tm_test_1(unsigned int tid);

void
tm_test_1_post(unsigned long nthreads, enum loop_mode loop,
               enum boundary_type btype, unsigned long long bound,
               int (*logmsg)(const char*, ...));
