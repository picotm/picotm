/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/config/picotm-arithmetic-config.h"
#include "picotm/compiler.h"
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include "picotm-arithmetic.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_arithmetic
 * \file
 *
 * \brief Transactional, safe arithmetics for native C types.
 */

/*
 * '_Bool'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE__BOOL) && \
            PICOTM_ARITHMETIC_HAVE_TYPE__BOOL || \
    defined(__PICOTM_DOXYGEN)

static inline
_Bool
__picotm_arithmetic_add__Bool(_Bool lhs, _Bool rhs)
{
    return lhs + rhs;
}

static inline
_Bool
__picotm_arithmetic_sub__Bool(_Bool lhs, _Bool rhs)
{
    return lhs - rhs;
}

static inline
_Bool
__picotm_arithmetic_mul__Bool(_Bool lhs, _Bool rhs)
{
    /* Logical 'and' avoids gcc's [-Werror=int-in-bool-context]. */
    return lhs && rhs;
}

static inline
_Bool
__picotm_arithmetic_div__Bool(_Bool lhs, _Bool rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_U_TX(_Bool, _Bool, 0, 1, 0,
                           __picotm_arithmetic_add__Bool,
                           __picotm_arithmetic_sub__Bool)
PICOTM_ARITHMETIC_SUB_U_TX(_Bool, _Bool, 0, 1, 0,
                           __picotm_arithmetic_add__Bool,
                           __picotm_arithmetic_sub__Bool)
PICOTM_ARITHMETIC_MUL_U_TX(_Bool, _Bool, 0, 1, 1, 0,
                           __picotm_arithmetic_mul__Bool,
                           __picotm_arithmetic_div__Bool)
PICOTM_ARITHMETIC_DIV_U_TX(_Bool, _Bool, 0, 1, 1, 0,
                           __picotm_arithmetic_mul__Bool,
                           __picotm_arithmetic_div__Bool)
/** \} */

#endif

/*
 * 'char'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_CHAR) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_CHAR || \
    defined(__PICOTM_DOXYGEN)

static inline
char
__picotm_arithmetic_add_char(char lhs, char rhs)
{
    return lhs + rhs;
}

static inline
char
__picotm_arithmetic_sub_char(char lhs, char rhs)
{
    return lhs - rhs;
}

static inline
char
__picotm_arithmetic_mul_char(char lhs, char rhs)
{
    return lhs * rhs;
}

static inline
char
__picotm_arithmetic_div_char(char lhs, char rhs)
{
#if CHAR_MIN == 0
    return lhs / rhs;
#else
    div_t res = div(lhs, rhs);
    return res.quot;
#endif
}

#if CHAR_MIN == 0
/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_U_TX(char, char, CHAR_MIN, CHAR_MAX, 0,
                           __picotm_arithmetic_add_char,
                           __picotm_arithmetic_sub_char)
PICOTM_ARITHMETIC_SUB_U_TX(char, char, CHAR_MIN, CHAR_MAX, 0,
                           __picotm_arithmetic_add_char,
                           __picotm_arithmetic_sub_char)
PICOTM_ARITHMETIC_MUL_U_TX(char, char, CHAR_MIN, CHAR_MAX, 1, 0,
                           __picotm_arithmetic_mul_char,
                           __picotm_arithmetic_div_char)
PICOTM_ARITHMETIC_DIV_U_TX(char, char, CHAR_MIN, CHAR_MAX, 1, 0,
                           __picotm_arithmetic_mul_char,
                           __picotm_arithmetic_div_char)
/** \} */
#else
/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_S_TX(char, char, CHAR_MIN, CHAR_MAX, 0,
                           __picotm_arithmetic_add_char,
                           __picotm_arithmetic_sub_char)
PICOTM_ARITHMETIC_SUB_S_TX(char, char, CHAR_MIN, CHAR_MAX, 0,
                           __picotm_arithmetic_add_char,
                           __picotm_arithmetic_sub_char)
PICOTM_ARITHMETIC_MUL_S_TX(char, char, CHAR_MIN, CHAR_MAX, 1, 0,
                           __picotm_arithmetic_mul_char,
                           __picotm_arithmetic_div_char)
PICOTM_ARITHMETIC_DIV_S_TX(char, char, CHAR_MIN, CHAR_MAX, 1, 0,
                           __picotm_arithmetic_mul_char,
                           __picotm_arithmetic_div_char)
/** \} */
#endif

#endif

/*
 * 'signed char'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_SIGNED_CHAR) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_SIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)

static inline
signed char
__picotm_arithmetic_add_schar(signed char lhs, signed char rhs)
{
    return lhs + rhs;
}

static inline
signed char
__picotm_arithmetic_sub_schar(signed char lhs, signed char rhs)
{
    return lhs - rhs;
}

static inline
signed char
__picotm_arithmetic_mul_schar(signed char lhs, signed char rhs)
{
    return lhs * rhs;
}

static inline
signed char
__picotm_arithmetic_div_schar(signed char lhs, signed char rhs)
{
    div_t res = div(lhs, rhs);
    return res.quot;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_S_TX(schar, signed char, SCHAR_MIN, SCHAR_MAX, 0,
                           __picotm_arithmetic_add_schar,
                           __picotm_arithmetic_sub_schar)
PICOTM_ARITHMETIC_SUB_S_TX(schar, signed char, SCHAR_MIN, SCHAR_MAX, 0,
                           __picotm_arithmetic_add_schar,
                           __picotm_arithmetic_sub_schar)
PICOTM_ARITHMETIC_MUL_S_TX(schar, signed char, SCHAR_MIN, SCHAR_MAX, 1, 0,
                           __picotm_arithmetic_mul_schar,
                           __picotm_arithmetic_div_schar)
PICOTM_ARITHMETIC_DIV_S_TX(schar, signed char, SCHAR_MIN, SCHAR_MAX, 1, 0,
                           __picotm_arithmetic_mul_schar,
                           __picotm_arithmetic_div_schar)
/** \} */

#endif

/*
 * 'short'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_SHORT) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_SHORT || \
    defined(__PICOTM_DOXYGEN)

static inline
short
__picotm_arithmetic_add_short(short lhs, short rhs)
{
    return lhs + rhs;
}

static inline
short
__picotm_arithmetic_sub_short(short lhs, short rhs)
{
    return lhs - rhs;
}

static inline
short
__picotm_arithmetic_mul_short(short lhs, short rhs)
{
    return lhs * rhs;
}

static inline
short
__picotm_arithmetic_div_short(short lhs, short rhs)
{
    div_t res = div(lhs, rhs);
    return res.quot;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_S_TX(short, short, SHRT_MIN, SHRT_MAX, 0,
                           __picotm_arithmetic_add_short,
                           __picotm_arithmetic_sub_short)
PICOTM_ARITHMETIC_SUB_S_TX(short, short, SHRT_MIN, SHRT_MAX, 0,
                           __picotm_arithmetic_add_short,
                           __picotm_arithmetic_sub_short)
PICOTM_ARITHMETIC_MUL_S_TX(short, short, SHRT_MIN, SHRT_MAX, 1, 0,
                           __picotm_arithmetic_mul_short,
                           __picotm_arithmetic_div_short)
PICOTM_ARITHMETIC_DIV_S_TX(short, short, SHRT_MIN, SHRT_MAX, 1, 0,
                           __picotm_arithmetic_mul_short,
                           __picotm_arithmetic_div_short)
/** \} */

#endif

/*
 * 'int'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_INT) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_INT || \
    defined(__PICOTM_DOXYGEN)

static inline
int
__picotm_arithmetic_add_int(int lhs, int rhs)
{
    return lhs + rhs;
}

static inline
int
__picotm_arithmetic_sub_int(int lhs, int rhs)
{
    return lhs - rhs;
}

static inline
int
__picotm_arithmetic_mul_int(int lhs, int rhs)
{
    return lhs * rhs;
}

static inline
int
__picotm_arithmetic_div_int(int lhs, int rhs)
{
    div_t res = div(lhs, rhs);
    return res.quot;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_S_TX(int, int, INT_MIN, INT_MAX, 0,
                           __picotm_arithmetic_add_int,
                           __picotm_arithmetic_sub_int)
PICOTM_ARITHMETIC_SUB_S_TX(int, int, INT_MIN, INT_MAX, 0,
                           __picotm_arithmetic_add_int,
                           __picotm_arithmetic_sub_int)
PICOTM_ARITHMETIC_MUL_S_TX(int, int, INT_MIN, INT_MAX, 1, 0,
                           __picotm_arithmetic_mul_int,
                           __picotm_arithmetic_div_int)
PICOTM_ARITHMETIC_DIV_S_TX(int, int, INT_MIN, INT_MAX, 1, 0,
                           __picotm_arithmetic_mul_int,
                           __picotm_arithmetic_div_int)
/** \} */

#endif

/*
 * 'long'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_LONG) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_LONG || \
    defined(__PICOTM_DOXYGEN)

static inline
long
__picotm_arithmetic_add_long(long lhs, long rhs)
{
    return lhs + rhs;
}

static inline
long
__picotm_arithmetic_sub_long(long lhs, long rhs)
{
    return lhs - rhs;
}

static inline
long
__picotm_arithmetic_mul_long(long lhs, long rhs)
{
    return lhs * rhs;
}

static inline
long
__picotm_arithmetic_div_long(long lhs, long rhs)
{
    ldiv_t res = ldiv(lhs, rhs);
    return res.quot;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_S_TX(long, long, LONG_MIN, LONG_MAX, 0l,
                           __picotm_arithmetic_add_long,
                           __picotm_arithmetic_sub_long)
PICOTM_ARITHMETIC_SUB_S_TX(long, long, LONG_MIN, LONG_MAX, 0l,
                           __picotm_arithmetic_add_long,
                           __picotm_arithmetic_sub_long)
PICOTM_ARITHMETIC_MUL_S_TX(long, long, LONG_MIN, LONG_MAX, 1l, 0l,
                           __picotm_arithmetic_mul_long,
                           __picotm_arithmetic_div_long)
PICOTM_ARITHMETIC_DIV_S_TX(long, long, LONG_MIN, LONG_MAX, 1l, 0l,
                           __picotm_arithmetic_mul_long,
                           __picotm_arithmetic_div_long)
/** \} */

#endif

/*
 * 'long long'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_LONG_LONG) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)

static inline
long long
__picotm_arithmetic_add_llong(long long lhs, long long rhs)
{
    return lhs + rhs;
}

static inline
long long
__picotm_arithmetic_sub_llong(long long lhs, long long rhs)
{
    return lhs - rhs;
}

static inline
long long
__picotm_arithmetic_mul_llong(long long lhs, long long rhs)
{
    return lhs * rhs;
}

static inline
long long
__picotm_arithmetic_div_llong(long long lhs, long long rhs)
{
    lldiv_t res = lldiv(lhs, rhs);
    return res.quot;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_S_TX(llong, long long, LLONG_MIN, LLONG_MAX, 0ll,
                           __picotm_arithmetic_add_llong,
                           __picotm_arithmetic_sub_llong)
PICOTM_ARITHMETIC_SUB_S_TX(llong, long long, LLONG_MIN, LLONG_MAX, 0ll,
                           __picotm_arithmetic_add_llong,
                           __picotm_arithmetic_sub_llong)
PICOTM_ARITHMETIC_MUL_S_TX(llong, long long, LLONG_MIN, LLONG_MAX, 1ll, 0ll,
                           __picotm_arithmetic_mul_llong,
                           __picotm_arithmetic_div_llong)
PICOTM_ARITHMETIC_DIV_S_TX(llong, long long, LLONG_MIN, LLONG_MAX, 1ll, 0ll,
                           __picotm_arithmetic_mul_llong,
                           __picotm_arithmetic_div_llong)
/** \} */

#endif

/*
 * 'unsigned char'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_CHAR) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_CHAR || \
    defined(__PICOTM_DOXYGEN)

static inline
unsigned char
__picotm_arithmetic_add_uchar(unsigned char lhs, unsigned char rhs)
{
    return lhs + rhs;
}

static inline
unsigned char
__picotm_arithmetic_sub_uchar(unsigned char lhs, unsigned char rhs)
{
    return lhs - rhs;
}

static inline
unsigned char
__picotm_arithmetic_mul_uchar(unsigned char lhs, unsigned char rhs)
{
    return lhs * rhs;
}

static inline
unsigned char
__picotm_arithmetic_div_uchar(unsigned char lhs, unsigned char rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_U_TX(uchar, unsigned char, 0u, UCHAR_MAX, 0u,
                           __picotm_arithmetic_add_uchar,
                           __picotm_arithmetic_sub_uchar)
PICOTM_ARITHMETIC_SUB_U_TX(uchar, unsigned char, 0u, UCHAR_MAX, 0u,
                           __picotm_arithmetic_add_uchar,
                           __picotm_arithmetic_sub_uchar)
PICOTM_ARITHMETIC_MUL_U_TX(uchar, unsigned char, 0u, UCHAR_MAX, 1u, 0u,
                           __picotm_arithmetic_mul_uchar,
                           __picotm_arithmetic_div_uchar)
PICOTM_ARITHMETIC_DIV_U_TX(uchar, unsigned char, 0u, UCHAR_MAX, 1u, 0u,
                           __picotm_arithmetic_mul_uchar,
                           __picotm_arithmetic_div_uchar)
/** \} */

#endif

/*
 * 'unsigned short'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_SHORT) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_SHORT || \
    defined(__PICOTM_DOXYGEN)

static inline
unsigned short
__picotm_arithmetic_add_ushort(unsigned short lhs, unsigned short rhs)
{
    return lhs + rhs;
}

static inline
unsigned short
__picotm_arithmetic_sub_ushort(unsigned short lhs, unsigned short rhs)
{
    return lhs - rhs;
}

static inline
unsigned short
__picotm_arithmetic_mul_ushort(unsigned short lhs, unsigned short rhs)
{
    return lhs * rhs;
}

static inline
unsigned short
__picotm_arithmetic_div_ushort(unsigned short lhs, unsigned short rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_U_TX(ushort, unsigned short, 0u, USHRT_MAX, 0u,
                           __picotm_arithmetic_add_ushort,
                           __picotm_arithmetic_sub_ushort)
PICOTM_ARITHMETIC_SUB_U_TX(ushort, unsigned short, 0u, USHRT_MAX, 0u,
                           __picotm_arithmetic_add_ushort,
                           __picotm_arithmetic_sub_ushort)
PICOTM_ARITHMETIC_MUL_U_TX(ushort, unsigned short, 0u, USHRT_MAX, 1u, 0u,
                           __picotm_arithmetic_mul_ushort,
                           __picotm_arithmetic_div_ushort)
PICOTM_ARITHMETIC_DIV_U_TX(ushort, unsigned short, 0u, USHRT_MAX, 1u, 0u,
                           __picotm_arithmetic_mul_ushort,
                           __picotm_arithmetic_div_ushort)
/** \} */

#endif

/*
 * 'unsigned int'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_INT) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_INT || \
    defined(__PICOTM_DOXYGEN)

static inline
unsigned int
__picotm_arithmetic_add_uint(unsigned int lhs, unsigned int rhs)
{
    return lhs + rhs;
}

static inline
unsigned int
__picotm_arithmetic_sub_uint(unsigned int lhs, unsigned int rhs)
{
    return lhs - rhs;
}

static inline
unsigned int
__picotm_arithmetic_mul_uint(unsigned int lhs, unsigned int rhs)
{
    return lhs * rhs;
}

static inline
unsigned int
__picotm_arithmetic_div_uint(unsigned int lhs, unsigned int rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_U_TX(uint, unsigned int, 0u, UINT_MAX, 0u,
                           __picotm_arithmetic_add_uint,
                           __picotm_arithmetic_sub_uint)
PICOTM_ARITHMETIC_SUB_U_TX(uint, unsigned int, 0u, UINT_MAX, 0u,
                           __picotm_arithmetic_add_uint,
                           __picotm_arithmetic_sub_uint)
PICOTM_ARITHMETIC_MUL_U_TX(uint, unsigned int, 0u, UINT_MAX, 1u, 0u,
                           __picotm_arithmetic_mul_uint,
                           __picotm_arithmetic_div_uint)
PICOTM_ARITHMETIC_DIV_U_TX(uint, unsigned int, 0u, UINT_MAX, 1u, 0u,
                           __picotm_arithmetic_mul_uint,
                           __picotm_arithmetic_div_uint)
/** \} */

#endif

/*
 * 'unsigned long'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_LONG) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_LONG || \
    defined(__PICOTM_DOXYGEN)

static inline
unsigned long
__picotm_arithmetic_add_ulong(unsigned long lhs, unsigned long rhs)
{
    return lhs + rhs;
}

static inline
unsigned long
__picotm_arithmetic_sub_ulong(unsigned long lhs, unsigned long rhs)
{
    return lhs - rhs;
}

static inline
unsigned long
__picotm_arithmetic_mul_ulong(unsigned long lhs, unsigned long rhs)
{
    return lhs * rhs;
}

static inline
unsigned long
__picotm_arithmetic_div_ulong(unsigned long lhs, unsigned long rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_U_TX(ulong, unsigned long, 0ul, ULONG_MAX, 0ul,
                           __picotm_arithmetic_add_ulong,
                           __picotm_arithmetic_sub_ulong)
PICOTM_ARITHMETIC_SUB_U_TX(ulong, unsigned long, 0ul, ULONG_MAX, 0ul,
                           __picotm_arithmetic_add_ulong,
                           __picotm_arithmetic_sub_ulong)
PICOTM_ARITHMETIC_MUL_U_TX(ulong, unsigned long, 0ul, ULONG_MAX, 1ul, 0ul,
                           __picotm_arithmetic_mul_ulong,
                           __picotm_arithmetic_div_ulong)
PICOTM_ARITHMETIC_DIV_U_TX(ulong, unsigned long, 0ul, ULONG_MAX, 1ul, 0ul,
                           __picotm_arithmetic_mul_ulong,
                           __picotm_arithmetic_div_ulong)
/** \} */

#endif

/*
 * 'unsigned long long'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_LONG_LONG) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_UNSIGNED_LONG_LONG || \
    defined(__PICOTM_DOXYGEN)

static inline
unsigned long long
__picotm_arithmetic_add_ullong(unsigned long long lhs, unsigned long long rhs)
{
    return lhs + rhs;
}

static inline
unsigned long long
__picotm_arithmetic_sub_ullong(unsigned long long lhs, unsigned long long rhs)
{
    return lhs - rhs;
}

static inline
unsigned long long
__picotm_arithmetic_mul_ullong(unsigned long long lhs, unsigned long long rhs)
{
    return lhs * rhs;
}

static inline
unsigned long long
__picotm_arithmetic_div_ullong(unsigned long long lhs, unsigned long long rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_U_TX(ullong, unsigned long long, 0ull, ULLONG_MAX, 0ull,
                           __picotm_arithmetic_add_ullong,
                           __picotm_arithmetic_sub_ullong)
PICOTM_ARITHMETIC_SUB_U_TX(ullong, unsigned long long, 0ull, ULLONG_MAX, 0ull,
                           __picotm_arithmetic_add_ullong,
                           __picotm_arithmetic_sub_ullong)
PICOTM_ARITHMETIC_DIV_U_TX(ullong, unsigned long long, 0ull, ULLONG_MAX, 1ull, 0ull,
                           __picotm_arithmetic_mul_ullong,
                           __picotm_arithmetic_div_ullong)
PICOTM_ARITHMETIC_MUL_U_TX(ullong, unsigned long long, 0ull, ULLONG_MAX, 1ull, 0ull,
                           __picotm_arithmetic_mul_ullong,
                           __picotm_arithmetic_div_ullong)
/** \} */

#endif

/*
 * 'float'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_FLOAT) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_FLOAT || \
    defined(__PICOTM_DOXYGEN)

static inline
float
__picotm_arithmetic_add_float(float lhs, float rhs)
{
    return lhs + rhs;
}

static inline
float
__picotm_arithmetic_sub_float(float lhs, float rhs)
{
    return lhs - rhs;
}

static inline
float
__picotm_arithmetic_mul_float(float lhs, float rhs)
{
    return lhs * rhs;
}

static inline
float
__picotm_arithmetic_div_float(float lhs, float rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_F_TX(float, float, FLT_MAX, 0.f,
                           __picotm_arithmetic_add_float,
                           __picotm_arithmetic_sub_float)
PICOTM_ARITHMETIC_SUB_F_TX(float, float, FLT_MAX, 0.f,
                           __picotm_arithmetic_add_float,
                           __picotm_arithmetic_sub_float)
PICOTM_ARITHMETIC_MUL_F_TX(float, float, FLT_MAX, 1.f, 0.f,
                           __picotm_arithmetic_mul_float,
                           __picotm_arithmetic_div_float)
PICOTM_ARITHMETIC_DIV_F_TX(float, float, FLT_MAX, 1.f, 0.f,
                           __picotm_arithmetic_mul_float,
                           __picotm_arithmetic_div_float)
/** \} */

#endif

/*
 * 'double'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_DOUBLE) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_DOUBLE || \
    defined(__PICOTM_DOXYGEN)

static inline
double
__picotm_arithmetic_add_double(double lhs, double rhs)
{
    return lhs + rhs;
}

static inline
double
__picotm_arithmetic_sub_double(double lhs, double rhs)
{
    return lhs - rhs;
}

static inline
double
__picotm_arithmetic_mul_double(double lhs, double rhs)
{
    return lhs * rhs;
}

static inline
double
__picotm_arithmetic_div_double(double lhs, double rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_F_TX(double, double, DBL_MAX, 0.,
                           __picotm_arithmetic_add_double,
                           __picotm_arithmetic_sub_double)
PICOTM_ARITHMETIC_SUB_F_TX(double, double, DBL_MAX, 0.,
                           __picotm_arithmetic_add_double,
                           __picotm_arithmetic_sub_double)
PICOTM_ARITHMETIC_MUL_F_TX(double, double, DBL_MAX, 1., 0.,
                           __picotm_arithmetic_mul_double,
                           __picotm_arithmetic_div_double)
PICOTM_ARITHMETIC_DIV_F_TX(double, double, DBL_MAX, 1., 0.,
                           __picotm_arithmetic_mul_double,
                           __picotm_arithmetic_div_double)
/** \} */

#endif

/*
 * 'long double'
 */

#if defined(PICOTM_ARITHMETIC_HAVE_TYPE_LONG_DOUBLE) && \
            PICOTM_ARITHMETIC_HAVE_TYPE_LONG_DOUBLE || \
    defined(__PICOTM_DOXYGEN)

static inline
long double
__picotm_arithmetic_add_ldouble(long double lhs, long double rhs)
{
    return lhs + rhs;
}

static inline
long double
__picotm_arithmetic_sub_ldouble(long double lhs, long double rhs)
{
    return lhs - rhs;
}

static inline
long double
__picotm_arithmetic_mul_ldouble(long double lhs, long double rhs)
{
    return lhs * rhs;
}

static inline
long double
__picotm_arithmetic_div_ldouble(long double lhs, long double rhs)
{
    return lhs / rhs;
}

/** \addtogroup group_arithmetic
 * \{ */
PICOTM_ARITHMETIC_ADD_F_TX(ldouble, long double, LDBL_MAX, 0.l,
                           __picotm_arithmetic_add_ldouble,
                           __picotm_arithmetic_sub_ldouble)
PICOTM_ARITHMETIC_SUB_F_TX(ldouble, long double, LDBL_MAX, 0.l,
                           __picotm_arithmetic_add_ldouble,
                           __picotm_arithmetic_sub_ldouble)
PICOTM_ARITHMETIC_MUL_F_TX(ldouble, long double, LDBL_MAX, 1.l, 0.l,
                           __picotm_arithmetic_mul_ldouble,
                           __picotm_arithmetic_div_ldouble)
PICOTM_ARITHMETIC_DIV_F_TX(ldouble, long double, LDBL_MAX, 1.l, 0.l,
                           __picotm_arithmetic_mul_ldouble,
                           __picotm_arithmetic_div_ldouble)
/** \} */

#endif

PICOTM_END_DECLS
