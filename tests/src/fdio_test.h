/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "test.h"

void fdio_test_1_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_1(unsigned int tid);
void fdio_test_1_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_2_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_2(unsigned int tid);
void fdio_test_2_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_3_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_3(unsigned int tid);
void fdio_test_3_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_4_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_4(unsigned int tid);
void fdio_test_4_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_5_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_5(unsigned int tid);
void fdio_test_5_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_6_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_6(unsigned int tid);
void fdio_test_6_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_7_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_7(unsigned int tid);
void fdio_test_7_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_8_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_8(unsigned int tid);
void fdio_test_8_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_9_pre(unsigned long nthreads, enum loop_mode loop,
                     enum boundary_type btype, unsigned long long bound,
                     int (*logmsg)(const char*, ...));
void fdio_test_9(unsigned int tid);
void fdio_test_9_post(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));

void fdio_test_10_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_10(unsigned int tid);
void fdio_test_10_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_11_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_11(unsigned int tid);
void fdio_test_11_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_12_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_12(unsigned int tid);
void fdio_test_12_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_13(unsigned int tid);

void fdio_test_14_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_14(unsigned int tid);
void fdio_test_14_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_15_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_15(unsigned int tid);
void fdio_test_15_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_16_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_16(unsigned int tid);
void fdio_test_16_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_17_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_17(unsigned int tid);
void fdio_test_17_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_18_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_18(unsigned int tid);
void fdio_test_18_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_19(unsigned int tid);

void fdio_test_20_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_20(unsigned int tid);
void fdio_test_20_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_21_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_21(unsigned int tid);
void fdio_test_21_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_22_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_22(unsigned int tid);
void fdio_test_22_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_23_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_23(unsigned int tid);
void fdio_test_23_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_24_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_24(unsigned int tid);
void fdio_test_24_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_25_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_25(unsigned int tid);
void fdio_test_25_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_26_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_26(unsigned int tid);
void fdio_test_26_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_27_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_27(unsigned int tid);
void fdio_test_27_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_28_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_28(unsigned int tid);
void fdio_test_28_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_29_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_29(unsigned int tid);
void fdio_test_29_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_30_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_30(unsigned int tid);
void fdio_test_30_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_31_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_31(unsigned int tid);
void fdio_test_31_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_32_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_32(unsigned int tid);
void fdio_test_32_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_33_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_33(unsigned int tid);
void fdio_test_33_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_34_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_34(unsigned int tid);
void fdio_test_34_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_35_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_35(unsigned int tid);
void fdio_test_35_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_36_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_36(unsigned int tid);
void fdio_test_36_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_37_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_37(unsigned int tid);
void fdio_test_37_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_38_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_38(unsigned int tid);
void fdio_test_38_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_39_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_39(unsigned int tid);
void fdio_test_39_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_40_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_40(unsigned int tid);
void fdio_test_40_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_41_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_41(unsigned int tid);
void fdio_test_41_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_42_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_42(unsigned int tid);
void fdio_test_42_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_43_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_43(unsigned int tid);
void fdio_test_43_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_44_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_44(unsigned int tid);
void fdio_test_44_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_45_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_45(unsigned int tid);
void fdio_test_45_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_46_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_46(unsigned int tid);
void fdio_test_46_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_47_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_47(unsigned int tid);
void fdio_test_47_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_48_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_48(unsigned int tid);
void fdio_test_48_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_49_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_49(unsigned int tid);
void fdio_test_49_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_50_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_50(unsigned int tid);
void fdio_test_50_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_51_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_51(unsigned int tid);
void fdio_test_51_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));

void fdio_test_52_pre(unsigned long nthreads, enum loop_mode loop,
                      enum boundary_type btype, unsigned long long bound,
                      int (*logmsg)(const char*, ...));
void fdio_test_52(unsigned int tid);
void fdio_test_52_post(unsigned long nthreads, enum loop_mode loop,
                       enum boundary_type btype, unsigned long long bound,
                       int (*logmsg)(const char*, ...));
