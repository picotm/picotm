/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/config/picotm-cast-config.h"
#include "picotm/compiler.h"
#include <float.h>
#include <limits.h>
#include "picotm-cast.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_cast
 * \file
 *
 * \brief Transactional, safe type casting for native C types.
 */

/*
 * Cast '_Bool'
 */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, llong, long long, LLONG_MIN, LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, double,  double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, float,   float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(_Bool, _Bool, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'char'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, _Bool, _Bool, 0, 1)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, schar, signed char , SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, llong, long long, LLONG_MIN, LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, uint,  unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, double, double, -DBL_MAX, DBL_MAX)
#endif
/** \} */
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(char, char, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'signed char'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, _Bool, _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, llong, long long, LLONG_MIN, LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, uint,  unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(schar, signed char, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'short'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, _Bool,  _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, long,  long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, llong, long long, LLONG_MIN, LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(short, short, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'int'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, _Bool, _Bool, 0, 1)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, llong, long long, LLONG_MIN, LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(int, int, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'long'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, _Bool, _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, llong, long long, LLONG_MIN, LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(long, long, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'long long'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, _Bool, _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(llong, long long, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'unsigned char'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, _Bool,  _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, llong, long long, LLONG_MIN,
               LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, ullong, unsigned long long, 0,
               ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uchar, unsigned char, ldouble, long double, -LDBL_MAX,
               LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'unsigned short'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, _Bool,  _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, schar, signed char, SCHAR_MIN,
               SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, short, short, SHRT_MIN,
               SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, llong, long long, LLONG_MIN,
               LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, ullong, unsigned long long, 0,
               ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ushort, unsigned short, ldouble, long double, -LDBL_MAX,
               LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'unsigned int'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, _Bool,  _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, llong, long long, LLONG_MIN,
               LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, ulong, unsigned long, 0, ULONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, ullong, unsigned long long, 0, ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(uint, unsigned int, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'unsigned long'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, _Bool,  _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, schar, signed char, SCHAR_MIN, SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, short, short, SHRT_MIN, SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, long, long, LONG_MIN, LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, llong, long long, LLONG_MIN,
               LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, ushort, unsigned short, 0, USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, ullong, unsigned long long, 0,
               ULLONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ulong, unsigned long, ldouble, long double, -LDBL_MAX,
               LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'unsigned long long'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)

/* to '_Bool'. */

#if defined(PICOTM_CAST_HAVE_TYPE__BOOL) && PICOTM_CAST_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, _Bool, _Bool, 0, 1)
/** \} */
#endif

/* to 'char'. */

#if defined(PICOTM_CAST_HAVE_TYPE_CHAR) && PICOTM_CAST_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, char, char, CHAR_MIN, CHAR_MAX)
/** \} */
#endif

/* to signed types. */

#if defined(PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, schar, signed char, SCHAR_MIN,
               SCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_SHORT) && PICOTM_CAST_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, short, short, SHRT_MIN,
               SHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_INT) && PICOTM_CAST_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, int, int, INT_MIN, INT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG) && PICOTM_CAST_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, long, long, LONG_MIN,
               LONG_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_LONG) && \
            PICOTM_CAST_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, llong, long long, LLONG_MIN,
               LLONG_MAX)
/** \} */
#endif

/* to unsigned types. */

#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, uchar, unsigned char, 0, UCHAR_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, ushort, unsigned short, 0,
               USHRT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, uint, unsigned int, 0, UINT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_CAST_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, ulong, unsigned long, 0,
               ULONG_MAX)
/** \} */
#endif

/* to floating-point types. */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif
#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ullong, unsigned long long, ldouble, long double,
               -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'double'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)

#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && \
            PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(double, double, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif

#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(double, double, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'float'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && \
            PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(float, float, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif

#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(float, float, ldouble, long double, -LDBL_MAX, LDBL_MAX)
/** \} */
#endif
#endif

/*
 * Cast 'double'
 */

#if defined(PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)

#if defined(PICOTM_CAST_HAVE_TYPE_FLOAT) && \
            PICOTM_CAST_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ldouble, long double, float, float, -FLT_MAX, FLT_MAX)
/** \} */
#endif

#if defined(PICOTM_CAST_HAVE_TYPE_DOUBLE) && \
            PICOTM_CAST_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_cast
 * \{ */
PICOTM_CAST_TX(ldouble, long double, double, double, -DBL_MAX, DBL_MAX)
/** \} */
#endif
#endif

PICOTM_END_DECLS
