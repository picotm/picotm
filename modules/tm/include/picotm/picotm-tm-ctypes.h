/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "picotm/config/picotm-tm-config.h"
#include "picotm/compiler.h"
#include "picotm-tm.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_tm
 * \file
 *
 * \brief Provides Transactional Memory operations for native C types.
 */

#if defined(PICOTM_TM_HAVE_TYPE__BOOL) && \
            PICOTM_TM_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(_Bool, _Bool)
PICOTM_TM_STORE_TX(_Bool, _Bool)
PICOTM_TM_PRIVATIZE_TX(_Bool, _Bool)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_CHAR) && \
            PICOTM_TM_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(char, char)
PICOTM_TM_STORE_TX(char, char)
PICOTM_TM_PRIVATIZE_TX(char, char)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_DOUBLE) && \
            PICOTM_TM_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(double, double)
PICOTM_TM_STORE_TX(double, double)
PICOTM_TM_PRIVATIZE_TX(double, double)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_FLOAT) && \
            PICOTM_TM_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(float, float)
PICOTM_TM_STORE_TX(float, float)
PICOTM_TM_PRIVATIZE_TX(float, float)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_INT) && \
            PICOTM_TM_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(int, int)
PICOTM_TM_STORE_TX(int, int)
PICOTM_TM_PRIVATIZE_TX(int, int)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_LONG) && \
            PICOTM_TM_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(long, long)
PICOTM_TM_STORE_TX(long, long)
PICOTM_TM_PRIVATIZE_TX(long, long)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_TM_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(ldouble, long double)
PICOTM_TM_STORE_TX(ldouble, long double)
PICOTM_TM_PRIVATIZE_TX(ldouble, long double)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_LONG_LONG) && \
            PICOTM_TM_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(llong, long long)
PICOTM_TM_STORE_TX(llong, long long)
PICOTM_TM_PRIVATIZE_TX(llong, long long)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_SHORT) && \
            PICOTM_TM_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(short, short)
PICOTM_TM_STORE_TX(short, short)
PICOTM_TM_PRIVATIZE_TX(short, short)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_TM_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(schar, signed char)
PICOTM_TM_STORE_TX(schar, signed char)
PICOTM_TM_PRIVATIZE_TX(schar, signed char)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_TM_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(uchar, unsigned char)
PICOTM_TM_STORE_TX(uchar, unsigned char)
PICOTM_TM_PRIVATIZE_TX(uchar, unsigned char)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_TM_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(uint, unsigned int)
PICOTM_TM_STORE_TX(uint, unsigned int)
PICOTM_TM_PRIVATIZE_TX(uint, unsigned int)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_TM_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(ulong, unsigned long)
PICOTM_TM_STORE_TX(ulong, unsigned long)
PICOTM_TM_PRIVATIZE_TX(ulong, unsigned long)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_TM_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(ullong, unsigned long long)
PICOTM_TM_STORE_TX(ullong, unsigned long long)
PICOTM_TM_PRIVATIZE_TX(ullong, unsigned long long)
#endif

#if defined(PICOTM_TM_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_TM_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_TM_LOAD_TX(ushort, unsigned short)
PICOTM_TM_STORE_TX(ushort, unsigned short)
PICOTM_TM_PRIVATIZE_TX(ushort, unsigned short)
#endif

PICOTM_END_DECLS
