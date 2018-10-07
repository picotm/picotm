/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "picotm/picotm.h"
#include "picotm/picotm-cast-ctypes.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include "ptr.h"
#include "safeblk.h"
#include "sysenv.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

#define UNUSED  __attribute__((unused))

/*
 * Test interfaces of <picotm/picotm-cast-ctypes.h>
 */

#define __TEST_SYMBOL(_func)    \
    test_ ## _func

/*
 * Test for success
 */

#define __TEST_SUCCESS_SYMBOL(_sname, _dname, _value)                   \
    __TEST_SYMBOL(success_ ## _sname ## _to_ ## _dname ## _ ## _value)

#define __TEST_SUCCESS_FUNC(_sym, _cond, _sname, _stype, _dname, _dtype,    \
                            _value)                                         \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        if (!(_cond)) {                                                     \
            tap_info("skipping next test for"                               \
                     " cast_" #_sname "_to_" #_dname "_tx(" #_value ");"    \
                     " condition failed");                                  \
            return;                                                         \
        }                                                                   \
        _stype value = (_value);                                            \
        picotm_begin                                                        \
            _dtype result =                                                 \
                cast_ ## _sname ## _to_ ## _dname ## _tx(value);            \
            if (result != value) {                                          \
                tap_error("mismatching value for cast from"                 \
                          " '" #_stype "' to '" #_dtype "'\n");             \
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;       \
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);  \
                picotm_error_mark_as_non_recoverable(&error);               \
                picotm_recover_from_error(&error);                          \
            }                                                               \
        picotm_commit                                                       \
            abort_transaction_on_error(__func__);                           \
        picotm_end                                                          \
    }

#define __TEST_SUCCESS_SKIP(_sym, _sname, _stype, _dname, _dtype, _value)   \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        tap_info("Skipping test for "                                       \
                 "cast_" #_sname "_to_" #_dname "_tx(" #_value ")");        \
    }

#define TEST_SUCCESS_FUNC_IF(_cond, _sname, _stype, _dname, _dtype, _value) \
    __TEST_SUCCESS_FUNC(                                                    \
        __TEST_SUCCESS_SYMBOL(_sname, _dname, value_ ## _value),            \
        _cond,                                                              \
        _sname, _stype,                                                     \
        _dname, _dtype,                                                     \
        _value)

#define TEST_SUCCESS_FUNC(_sname, _stype, _dname, _dtype, _value)   \
    __TEST_SUCCESS_FUNC(                                            \
        __TEST_SUCCESS_SYMBOL(_sname, _dname, value_ ## _value),    \
        true,                                                       \
        _sname, _stype,                                             \
        _dname, _dtype,                                             \
        _value)

#define TEST_SUCCESS_SKIP(_sname, _stype, _dname, _dtype, _value)   \
    __TEST_SUCCESS_SKIP(                                            \
        __TEST_SUCCESS_SYMBOL(_sname, _dname, value_ ## _value),    \
        _sname, _stype,                                             \
        _dname, _dtype,                                             \
        _value)

#define TEST_SUCCESS(_sname, _dname, _value)                        \
    {"Test cast_" #_sname "_to_" #_dname "_tx(" #_value ")",        \
        __TEST_SUCCESS_SYMBOL(_sname, _dname, value_ ## _value),    \
        NULL, NULL}

/*
 * Test for error
 */

#define __TEST_ERROR_SYMBOL(_error, _sname, _dname, _value)             \
    __TEST_SYMBOL(                                                      \
        e_ ## _error ## _ ## _sname ## _to_ ## _dname ## _ ## _value)

/* Test for errno code */

#define __TEST_E_ERRNO_FUNC(_sym, _cond, _errno,                            \
                            _sname, _stype, _dname, _dtype, _value)         \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        if (!(_cond)) {                                                     \
            tap_info("skipping next test for"                               \
                     " cast_" #_sname "_to_" #_dname "_tx(" #_value ");"    \
                     " condition failed");                                  \
            return;                                                         \
        }                                                                   \
        bool error_detected = false;                                        \
        errno = 0;                                                          \
        picotm_begin                                                        \
            _dtype result UNUSED =                                          \
                cast_ ## _sname ## _to_ ## _dname ## _tx(_value);           \
        picotm_commit                                                       \
            if ((picotm_error_status() == PICOTM_ERRNO) &&                  \
                picotm_error_as_errno() == (_errno)) {                      \
                error_detected = true;                                      \
            }                                                               \
        picotm_end                                                          \
        if (!error_detected) {                                              \
            tap_error("%s, No error detected.", __func__);                  \
            abort_safe_block();                                             \
        }                                                                   \
    }

#define __TEST_E_ERRNO_SKIP(_sym, _errno,                               \
                            _sname, _stype, _dname, _dtype, _value)     \
    static void                                                         \
    _sym(unsigned int tid)                                              \
    {                                                                   \
        tap_info("Skipping errno (" #_errno ") test for "               \
                 "cast_" #_sname "_to_" #_dname "_tx(" #_value ")");    \
    }

#define TEST_E_ERRNO_FUNC_IF(_cond, _errno, _sname, _stype, _dname, _dtype, \
                             _value)                                        \
    __TEST_E_ERRNO_FUNC(__TEST_ERROR_SYMBOL(errno_ ## _errno,               \
                                            _sname, _dname,                 \
                                            value_ ## _value),              \
                        _cond,                                              \
                        _errno,                                             \
                        _sname, _stype,                                     \
                        _dname, _dtype,                                     \
                        _value)

#define TEST_E_ERRNO_FUNC(_errno, _sname, _stype, _dname, _dtype, _value)   \
    __TEST_E_ERRNO_FUNC(__TEST_ERROR_SYMBOL(errno_ ## _errno,               \
                                            _sname, _dname,                 \
                                            value_ ## _value),              \
                        true,                                               \
                        _errno,                                             \
                        _sname, _stype,                                     \
                        _dname, _dtype,                                     \
                        _value)

#define TEST_E_ERRNO_SKIP(_errno, _sname, _stype, _dname, _dtype, _value)   \
    __TEST_E_ERRNO_SKIP(__TEST_ERROR_SYMBOL(errno_ ## _errno,               \
                                            _sname, _dname,                 \
                                            value_ ## _value),              \
                        _errno,                                             \
                        _sname, _stype,                                     \
                        _dname, _dtype,                                     \
                        _value)

#define TEST_E_ERRNO(_errno, _sname, _dname, _value)                        \
    {"Test cast_" #_sname "_to_" #_dname "_tx(" #_value ") for " #_errno,   \
        __TEST_ERROR_SYMBOL(errno_ ## _errno, _sname, _dname,               \
                            value_ ## _value),                              \
        NULL, NULL}

/*
 * Test constants
 */

#define NEG_ONE (-1)
#define POS_TWO (2)

#if (SHRT_MIN < SCHAR_MIN)
#define SCHAR_MIN_SUB_ONE   ((short)SCHAR_MIN - 1)
#elif (INT_MIN < SCHAR_MIN)
#define SCHAR_MIN_SUB_ONE   ((int)SCHAR_MIN - 1)
#elif (LONG_MIN < SCHAR_MIN)
#define SCHAR_MIN_SUB_ONE   ((long)SCHAR_MIN - 1l)
#elif (LLONG_MIN < SCHAR_MIN)
#define SCHAR_MIN_SUB_ONE   ((long long)SCHAR_MIN - 1ll)
#endif

#if (SHRT_MAX > SCHAR_MAX)
#define SCHAR_MAX_ADD_ONE   ((short)SCHAR_MAX + 1)
#elif (INT_MAX > SCHAR_MAX)
#define SCHAR_MAX_ADD_ONE   ((int)SCHAR_MAX + 1)
#elif (LONG_MAX > SCHAR_MAX)
#define SCHAR_MAX_ADD_ONE   ((long)SCHAR_MAX + 1l)
#elif (LLONG_MAX > SCHAR_MAX)
#define SCHAR_MAX_ADD_ONE   ((long long)SCHAR_MAX + 1ll)
#endif

#if (SCHAR_MAX < UCHAR_MAX)
#define SCHAR_MAX_ADD_ONE_U  ((unsigned char)SCHAR_MAX + 1)
#endif

#if (INT_MIN < SHRT_MIN)
#define SHRT_MIN_SUB_ONE    ((int)SHRT_MIN - 1)
#elif (LONG_MIN < SHRT_MIN)
#define SHRT_MIN_SUB_ONE    ((long)SHRT_MIN - 1l)
#elif (LLONG_MIN < SHRT_MIN)
#define SHRT_MIN_SUB_ONE    ((long long)SHRT_MIN - 1ll)
#endif

#if (INT_MAX > SHRT_MAX)
#define SHRT_MAX_ADD_ONE    ((int)SHRT_MAX + 1)
#elif (LONG_MAX > SHRT_MAX)
#define SHRT_MAX_ADD_ONE    ((long)SHRT_MAX + 1l)
#elif (LLONG_MAX > SHRT_MAX)
#define SHRT_MAX_ADD_ONE    ((long long)SHRT_MAX + 1ll)
#endif

#if (SHRT_MAX < USHRT_MAX)
#define SHRT_MAX_ADD_ONE_U  ((unsigned short)SHRT_MAX + 1)
#endif

#if (LONG_MIN < INT_MIN)
#define INT_MIN_SUB_ONE     ((long)INT_MIN - 1l)
#elif (LLONG_MIN < INT_MIN)
#define INT_MIN_SUB_ONE     ((long long)INT_MIN - 1ll)
#endif

#if (LONG_MAX > INT_MAX)
#define INT_MAX_ADD_ONE     ((long)INT_MAX + 1l)
#elif (LLONG_MAX > INT_MAX)
#define INT_MAX_ADD_ONE     ((long long)INT_MAX + 1ll)
#endif

#if (INT_MAX < UINT_MAX)
#define INT_MAX_ADD_ONE_U   ((unsigned int)INT_MAX + 1u)
#endif

#if (LLONG_MIN < LONG_MIN)
#define LONG_MIN_SUB_ONE    ((long long)LONG_MIN - 1ll)
#endif

#if (LLONG_MAX > LONG_MAX)
#define LONG_MAX_ADD_ONE    ((long long)LONG_MAX + 1ll)
#endif

#if (LONG_MAX < ULONG_MAX)
#define LONG_MAX_ADD_ONE_U  ((unsigned long)LONG_MAX + 1ul)
#endif

#if (LLONG_MAX < ULLONG_MAX)
#define LLONG_MAX_ADD_ONE_U ((unsigned long long)LLONG_MAX + 1ull)
#endif

#if (USHRT_MAX > UCHAR_MAX)
#define UCHAR_MAX_ADD_ONE   ((unsigned short)UCHAR_MAX + 1)
#elif (UINT_MAX > UCHAR_MAX)
#define UCHAR_MAX_ADD_ONE   ((unsigned int)UCHAR_MAX + 1u)
#elif (ULONG_MAX > UCHAR_MAX)
#define UCHAR_MAX_ADD_ONE   ((unsigned long)UCHAR_MAX + 1ul)
#elif (ULLONG_MAX > UCHAR_MAX)
#define UCHAR_MAX_ADD_ONE   ((unsigned long long)UCHAR_MAX + 1ull)
#endif

#if (UINT_MAX > USHRT_MAX)
#define USHRT_MAX_ADD_ONE   ((unsigned int)USHRT_MAX + 1u)
#elif (ULONG_MAX > USHRT_MAX)
#define USHRT_MAX_ADD_ONE   ((unsigned long)USHRT_MAX + 1ul)
#elif (ULLONG_MAX > USHRT_MAX)
#define USHRT_MAX_ADD_ONE   ((unsigned long long)USHRT_MAX + 1ull)
#endif

#if (ULONG_MAX > UINT_MAX)
#define UINT_MAX_ADD_ONE    ((unsigned long)UINT_MAX + 1ul)
#elif (ULLONG_MAX > UINT_MAX)
#define UINT_MAX_ADD_ONE    ((unsigned long long)UINT_MAX + 1ull)
#endif

#if (ULLONG_MAX > ULONG_MAX)
#define ULONG_MAX_ADD_ONE   ((unsigned long long)ULONG_MAX + 1ull)
#endif

#define NEG_FLT_MAX         (-FLT_MAX)
#define NEG_FLT_MAX_MUL_TWO (((double)NEG_FLT_MAX) * 2.)
#define     FLT_MAX_MUL_TWO (((double)    FLT_MAX) * 2.)

#define NEG_DBL_MAX         (-DBL_MAX)
#define NEG_DBL_MAX_MUL_TWO (((long double)NEG_DBL_MAX) * 2.l)
#define     DBL_MAX_MUL_TWO (((long double)    DBL_MAX) * 2.l)

#define NEG_LDBL_MAX        (-LDBL_MAX)

/*
 * Cast from '_Bool'
 */

TEST_SUCCESS_FUNC(_Bool, _Bool, char, char, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, char, char, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, schar, signed char, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, schar, signed char, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, short, short, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, short, short, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, int, int, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, int, int, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, long, long, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, long, long, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, llong, long long, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, llong, long long, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, uchar, unsigned char, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, uchar, unsigned char, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, ushort, unsigned short, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, ushort, unsigned short, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, uint, unsigned int, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, uint, unsigned int, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, ulong, unsigned long, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, ulong, unsigned long, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, ullong, unsigned long long, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, ullong, unsigned long long, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, double, double, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, double, double, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, float, float, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, float, float, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, ldouble, long double, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, ldouble, long double, true)

/*
 * Cast from 'char'
 */

TEST_SUCCESS_FUNC(char, char, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(char, char, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(char, char, schar, signed char, 0)
TEST_SUCCESS_FUNC(char, char, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(char, char, short, short, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, short, short, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, int, int, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, int, int, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, long, long, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, long, long, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, llong, long long, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, llong, long long, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(char, char, uchar, unsigned char, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(char, char, ushort, unsigned short, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(char, char, uint, unsigned int, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(char, char, ulong, unsigned long, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(char, char, ullong, unsigned long long, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, double, double, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, double, double, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, float, float, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, float, float, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, ldouble, long double, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, ldouble, long double, CHAR_MAX)

TEST_E_ERRNO_FUNC(ERANGE, char, char, _Bool, _Bool, NEG_ONE)
TEST_E_ERRNO_FUNC(ERANGE, char, char, _Bool, _Bool, POS_TWO)

/*
 * Cast from 'signed char'
 */

TEST_SUCCESS_FUNC(schar, signed char, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(schar, signed char, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(schar, signed char, char, char, 0)
TEST_SUCCESS_FUNC(schar, signed char, char, char, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, short, short, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, short, short, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, int, int, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, int, int, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, long, long, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, long, long, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, llong, long long, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, llong, long long, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(schar, signed char, uchar, unsigned char, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(schar, signed char, ushort, unsigned short, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(schar, signed char, uint, unsigned int, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(schar, signed char, ulong, unsigned long, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(schar, signed char, ullong, unsigned long long, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, double, double, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, double, double, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, float, float, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, float, float, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, ldouble, long double, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, ldouble, long double, SCHAR_MAX)

TEST_E_ERRNO_FUNC(ERANGE, schar, signed char, _Bool, _Bool, NEG_ONE)
TEST_E_ERRNO_FUNC(ERANGE, schar, signed char, _Bool, _Bool, POS_TWO)

/*
 * Cast from 'short'
 */

TEST_SUCCESS_FUNC(short, short, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(short, short, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(short, short, char, char, CHAR_MIN)
TEST_SUCCESS_FUNC(short, short, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(short, short, schar, signed char, SCHAR_MIN)
TEST_SUCCESS_FUNC(short, short, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(short, short, int, int, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short, int, int, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, long, long, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short, long, long, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, llong, long long, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short, llong, long long, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(short, short, uchar, unsigned char, CHAR_MAX)
TEST_SUCCESS_FUNC(short, short, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(short, short, ushort, unsigned short, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(short, short, uint, unsigned int, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(short, short, ulong, unsigned long, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(short, short, ullong, unsigned long long, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, double, double, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short, double, double, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, float, float, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short, float, float, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, ldouble, long double, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short, ldouble, long double, SHRT_MAX)

TEST_E_ERRNO_FUNC(ERANGE, short, short, _Bool, _Bool, NEG_ONE)
TEST_E_ERRNO_FUNC(ERANGE, short, short, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, short, short, schar, signed char, SCHAR_MIN_SUB_ONE)
TEST_E_ERRNO_FUNC(ERANGE, short, short, schar, signed char, SCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, short, short, uchar, unsigned char, UCHAR_MAX_ADD_ONE)

/*
 * Cast from 'int'
 */

TEST_SUCCESS_FUNC(int, int, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(int, int, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(int, int, char, char, CHAR_MIN)
TEST_SUCCESS_FUNC(int, int, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(int, int, schar, signed char, SCHAR_MIN)
TEST_SUCCESS_FUNC(int, int, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(int, int, short, short, SHRT_MIN)
TEST_SUCCESS_FUNC(int, int, short, short, SHRT_MAX)
TEST_SUCCESS_FUNC(int, int, long, long, INT_MIN)
TEST_SUCCESS_FUNC(int, int, long, long, INT_MAX)
TEST_SUCCESS_FUNC(int, int, llong, long long, INT_MIN)
TEST_SUCCESS_FUNC(int, int, llong, long long, INT_MAX)
TEST_SUCCESS_FUNC(int, int, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(int, int, uchar, unsigned char, CHAR_MAX)
TEST_SUCCESS_FUNC(int, int, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(int, int, ushort, unsigned short, SHRT_MAX)
TEST_SUCCESS_FUNC(int, int, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(int, int, uint, unsigned int, INT_MAX)
TEST_SUCCESS_FUNC(int, int, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(int, int, ulong, unsigned long, INT_MAX)
TEST_SUCCESS_FUNC(int, int, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(int, int, ullong, unsigned long long, INT_MAX)
TEST_SUCCESS_FUNC(int, int, double, double, INT_MIN)
TEST_SUCCESS_FUNC(int, int, double, double, INT_MAX)
TEST_SUCCESS_FUNC(int, int, float, float, INT_MIN)
TEST_SUCCESS_FUNC(int, int, float, float, INT_MAX)
TEST_SUCCESS_FUNC(int, int, ldouble, long double, INT_MIN)
TEST_SUCCESS_FUNC(int, int, ldouble, long double, INT_MAX)

TEST_E_ERRNO_FUNC(ERANGE, int, int, _Bool, _Bool, NEG_ONE)
TEST_E_ERRNO_FUNC(ERANGE, int, int, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, int, int, schar, signed char, SCHAR_MIN_SUB_ONE)
TEST_E_ERRNO_FUNC(ERANGE, int, int, schar, signed char, SCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, int, int, short, short, SHRT_MIN_SUB_ONE)
TEST_E_ERRNO_FUNC(ERANGE, int, int, short, short, SHRT_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, int, int, uchar, unsigned char, UCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, int, int, ushort, unsigned short, USHRT_MAX_ADD_ONE)

/*
 * Cast from 'long'
 */

TEST_SUCCESS_FUNC(long, long, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(long, long, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(long, long, char, char, CHAR_MIN)
TEST_SUCCESS_FUNC(long, long, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(long, long, schar, signed char, SCHAR_MIN)
TEST_SUCCESS_FUNC(long, long, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(long, long, short, short, SHRT_MIN)
TEST_SUCCESS_FUNC(long, long, short, short, SHRT_MAX)
TEST_SUCCESS_FUNC(long, long, int, int, INT_MIN)
TEST_SUCCESS_FUNC(long, long, int, int, INT_MAX)
TEST_SUCCESS_FUNC(long, long, llong, long long, LONG_MIN)
TEST_SUCCESS_FUNC(long, long, llong, long long, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(long, long, uchar, unsigned char, CHAR_MAX)
TEST_SUCCESS_FUNC(long, long, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(long, long, ushort, unsigned short, SHRT_MAX)
TEST_SUCCESS_FUNC(long, long, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(long, long, uint, unsigned int, INT_MAX)
TEST_SUCCESS_FUNC(long, long, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(long, long, ulong, unsigned long, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(long, long, ullong, unsigned long long, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, double, double, LONG_MIN)
TEST_SUCCESS_FUNC(long, long, double, double, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, float, float, LONG_MIN)
TEST_SUCCESS_FUNC(long, long, float, float, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, ldouble, long double, LONG_MIN)
TEST_SUCCESS_FUNC(long, long, ldouble, long double, LONG_MAX)

TEST_E_ERRNO_FUNC(ERANGE, long, long, _Bool, _Bool, NEG_ONE)
TEST_E_ERRNO_FUNC(ERANGE, long, long, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, long, long, schar, signed char, SCHAR_MIN_SUB_ONE)
TEST_E_ERRNO_FUNC(ERANGE, long, long, schar, signed char, SCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, long, long, short, short, SHRT_MIN_SUB_ONE)
TEST_E_ERRNO_FUNC(ERANGE, long, long, short, short, SHRT_MAX_ADD_ONE)
#if (LONG_MIN < INT_MIN)
TEST_E_ERRNO_FUNC(ERANGE, long, long, int, int, INT_MIN_SUB_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, long, long, int, int, INT_MIN_SUB_ONE)
#endif
#if (LONG_MAX > INT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, long, long, int, int, INT_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, long, long, int, int, INT_MAX_ADD_ONE)
#endif
TEST_E_ERRNO_FUNC(ERANGE, long, long, uchar, unsigned char, UCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, long, long, ushort, unsigned short, USHRT_MAX_ADD_ONE)
#if (LONG_MAX > UINT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, long, long, uint, unsigned int, UINT_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, long, long, uint, unsigned int, UINT_MAX_ADD_ONE)
#endif

/*
 * Cast from 'long long'
 */

TEST_SUCCESS_FUNC(llong, long long, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(llong, long long, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(llong, long long, char, char, CHAR_MIN)
TEST_SUCCESS_FUNC(llong, long long, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(llong, long long, schar, signed char, SCHAR_MIN)
TEST_SUCCESS_FUNC(llong, long long, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(llong, long long, short, short, SHRT_MIN)
TEST_SUCCESS_FUNC(llong, long long, short, short, SHRT_MAX)
TEST_SUCCESS_FUNC(llong, long long, int, int, INT_MIN)
TEST_SUCCESS_FUNC(llong, long long, int, int, INT_MAX)
TEST_SUCCESS_FUNC(llong, long long, long, long, LONG_MIN)
TEST_SUCCESS_FUNC(llong, long long, long, long, LONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(llong, long long, uchar, unsigned char, CHAR_MAX)
TEST_SUCCESS_FUNC(llong, long long, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(llong, long long, ushort, unsigned short, SHRT_MAX)
TEST_SUCCESS_FUNC(llong, long long, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(llong, long long, uint, unsigned int, UINT_MAX)
TEST_SUCCESS_FUNC(llong, long long, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(llong, long long, ulong, unsigned long, LONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(llong, long long, ullong, unsigned long long, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, double, double, LLONG_MIN)
TEST_SUCCESS_FUNC(llong, long long, double, double, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, float, float, LLONG_MIN)
TEST_SUCCESS_FUNC(llong, long long, float, float, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, ldouble, long double, LLONG_MIN)
TEST_SUCCESS_FUNC(llong, long long, ldouble, long double, LLONG_MAX)

TEST_E_ERRNO_FUNC(ERANGE, llong, long long, _Bool, _Bool, NEG_ONE)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, schar, signed char, SCHAR_MIN_SUB_ONE)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, schar, signed char, SCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, short, short, SHRT_MIN_SUB_ONE)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, short, short, SHRT_MAX_ADD_ONE)
#if (LLONG_MIN < INT_MIN)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, int, int, INT_MIN_SUB_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, llong, long long, int, int, INT_MIN_SUB_ONE)
#endif
#if (LLONG_MAX > INT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, int, int, INT_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, llong, long long, int, int, INT_MAX_ADD_ONE)
#endif
#if (LLONG_MIN < LONG_MIN)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, long, long, LONG_MIN_SUB_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, llong, long long, long, long, LONG_MIN_SUB_ONE)
#endif
#if (LLONG_MAX > LONG_MAX)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, long, long, LONG_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, llong, long long, long, long, LONG_MAX_ADD_ONE)
#endif
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, uchar, unsigned char, UCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, ushort, unsigned short, USHRT_MAX_ADD_ONE)
#if (LLONG_MAX > UINT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, uint, unsigned int, UINT_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, llong, long long, uint, unsigned int, UINT_MAX_ADD_ONE)
#endif
#if (LLONG_MAX > ULONG_MAX)
TEST_E_ERRNO_FUNC(ERANGE, llong, long long, ulong, unsigned long, ULONG_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, llong, long long, ulong, unsigned long, ULONG_MAX_ADD_ONE)
#endif

/*
 * Cast from 'unsigned char'
 */

TEST_SUCCESS_FUNC(uchar, unsigned char, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(uchar, unsigned char, char, char, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, schar, signed char, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, short, short, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, short, short, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, int, int, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, int, int, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, long, long, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, long, long, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, llong, long long, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, llong, long long, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, ushort, unsigned short, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, uint, unsigned int, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, ulong, unsigned long, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, ullong, unsigned long long, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, double, double, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, double, double, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, float, float, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, float, float, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, ldouble, long double, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, ldouble, long double, UCHAR_MAX)

TEST_E_ERRNO_FUNC(ERANGE, uchar, unsigned char, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, uchar, unsigned char, schar, signed char, SCHAR_MAX_ADD_ONE_U)

/*
 * Cast from 'unsigned short'
 */

TEST_SUCCESS_FUNC(ushort, unsigned short, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(ushort, unsigned short, char, char, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, schar, signed char, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, short, short, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, short, short, SHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, int, int, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, int, int, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, long, long, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, long, long, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, llong, long long, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, llong, long long, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, uchar, unsigned char, UCHAR_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, uint, unsigned int, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, ulong, unsigned long, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, ullong, unsigned long long, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, double, double, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, double, double, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, float, float, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, float, float, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, ldouble, long double, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, ldouble, long double, USHRT_MAX)

TEST_E_ERRNO_FUNC(ERANGE, ushort, unsigned short, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, ushort, unsigned short, schar, signed char, SCHAR_MAX_ADD_ONE_U)
TEST_E_ERRNO_FUNC(ERANGE, ushort, unsigned short, short, short, SHRT_MAX_ADD_ONE_U)
TEST_E_ERRNO_FUNC(ERANGE, ushort, unsigned short, uchar, unsigned char, UCHAR_MAX_ADD_ONE)

/*
 * Cast from 'unsigned int'
 */

TEST_SUCCESS_FUNC(uint, unsigned int, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(uint, unsigned int, char, char, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, schar, signed char, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, short, short, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, short, short, SHRT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, int, int, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, int, int, INT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, long, long, 0)
#if (UINT_MAX < LONG_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, long, long, UINT_MAX)
#else
TEST_SUCCESS_SKIP(uint, unsigned int, long, long, UINT_MAX)
#endif
TEST_SUCCESS_FUNC(uint, unsigned int, llong, long long, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, llong, long long, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, uchar, unsigned char, UCHAR_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, ushort, unsigned short, USHRT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, ulong, unsigned long, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, ullong, unsigned long long, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, double, double, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, double, double, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, float, float, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, float, float, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, ldouble, long double, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, ldouble, long double, UINT_MAX)

TEST_E_ERRNO_FUNC(ERANGE, uint, unsigned int, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, uint, unsigned int, schar, signed char, SCHAR_MAX_ADD_ONE_U)
TEST_E_ERRNO_FUNC(ERANGE, uint, unsigned int, short, short, SHRT_MAX_ADD_ONE_U)
#if (UINT_MAX > INT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, uint, unsigned int, int, int, INT_MAX_ADD_ONE_U)
#else
TEST_E_ERRNO_SKIP(ERANGE, uint, unsigned int, int, int, INT_MAX_ADD_ONE_U)
#endif
TEST_E_ERRNO_FUNC(ERANGE, uint, unsigned int, uchar, unsigned char, UCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, uint, unsigned int, ushort, unsigned short, USHRT_MAX_ADD_ONE)

/*
 * Cast from 'unsigned long'
 */

TEST_SUCCESS_FUNC(ulong, unsigned long, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(ulong, unsigned long, char, char, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, schar, signed char, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, short, short, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, short, short, SHRT_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, int, int, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, int, int, INT_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, long, long, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, long, long, LONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, llong, long long, 0)
#if (ULONG_MAX > LLONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, llong, long long, LLONG_MAX)
#else
TEST_SUCCESS_SKIP(ulong, unsigned long, llong, long long, LLONG_MAX)
#endif
TEST_SUCCESS_FUNC(ulong, unsigned long, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, uchar, unsigned char, UCHAR_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, ushort, unsigned short, USHRT_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, uint, unsigned int, UINT_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, ullong, unsigned long long, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, ullong, unsigned long long, ULONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, double, double, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, double, double, ULONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, float, float, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, float, float, ULONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, ldouble, long double, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, ldouble, long double, ULONG_MAX)

TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, schar, signed char, SCHAR_MAX_ADD_ONE_U)
TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, short, short, SHRT_MAX_ADD_ONE_U)
#if (ULONG_MAX > INT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, int, int, INT_MAX_ADD_ONE_U)
#else
TEST_E_ERRNO_SKIP(ERANGE, ulong, unsigned long, int, int, INT_MAX_ADD_ONE_U)
#endif
#if (ULONG_MAX > LONG_MAX)
TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, long, long, LONG_MAX_ADD_ONE_U)
#else
TEST_E_ERRNO_SKIP(ERANGE, ulong, unsigned long, long, long, LONG_MAX_ADD_ONE_U)
#endif
TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, uchar, unsigned char, UCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, ushort, unsigned short, USHRT_MAX_ADD_ONE)
#if (ULONG_MAX > UINT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, ulong, unsigned long, uint, unsigned int, UINT_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, ulong, unsigned long, uint, unsigned int, UINT_MAX_ADD_ONE)
#endif

/*
 * Cast from 'unsigned long long'
 */

TEST_SUCCESS_FUNC(ullong, unsigned long long, _Bool, _Bool, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, _Bool, _Bool, 1)
TEST_SUCCESS_FUNC(ullong, unsigned long long, char, char, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, char, char, CHAR_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, schar, signed char, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, schar, signed char, SCHAR_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, short, short, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, short, short, SHRT_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, int, int, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, int, int, INT_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, long, long, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, long, long, LONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, llong, long long, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, llong, long long, LLONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, uchar, unsigned char, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, uchar, unsigned char, UCHAR_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, ushort, unsigned short, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, ushort, unsigned short, USHRT_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, uint, unsigned int, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, uint, unsigned int, UINT_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, ulong, unsigned long, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, ulong, unsigned long, ULONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, double, double, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, double, double, ULLONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, float, float, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, float, float, ULLONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, ldouble, long double, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, ldouble, long double, ULLONG_MAX)

TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, _Bool, _Bool, POS_TWO)
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, schar, signed char, SCHAR_MAX_ADD_ONE_U)
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, short, short, SHRT_MAX_ADD_ONE_U)
#if (ULLONG_MAX > INT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, int, int, INT_MAX_ADD_ONE_U)
#else
TEST_E_ERRNO_SKIP(ERANGE, ullong, unsigned long long, int, int, INT_MAX_ADD_ONE_U)
#endif
#if (ULLONG_MAX > LONG_MAX)
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, long, long, LONG_MAX_ADD_ONE_U)
#else
TEST_E_ERRNO_SKIP(ERANGE, ullong, unsigned long long, long, long, LONG_MAX_ADD_ONE_U)
#endif
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, uchar, unsigned char, UCHAR_MAX_ADD_ONE)
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, ushort, unsigned short, USHRT_MAX_ADD_ONE)
#if (ULLONG_MAX > UINT_MAX)
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, uint, unsigned int, UINT_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKP(ERANGE, ullong, unsigned long long, uint, unsigned int, UINT_MAX_ADD_ONE)
#endif
#if (ULLONG_MAX > ULONG_MAX)
TEST_E_ERRNO_FUNC(ERANGE, ullong, unsigned long long, ulong, unsigned long, ULONG_MAX_ADD_ONE)
#else
TEST_E_ERRNO_SKIP(ERANGE, ullong, unsigned long long, ulong, unsigned long, ULONG_MAX_ADD_ONE)
#endif

/*
 * Cast from 'float'
 */

TEST_SUCCESS_FUNC(                   float, float, double,  double,      NEG_FLT_MAX)
TEST_SUCCESS_FUNC(                   float, float, double,  double,          FLT_MAX)
TEST_SUCCESS_FUNC_IF(!is_valgrind(), float, float, ldouble, long double, NEG_FLT_MAX)
TEST_SUCCESS_FUNC_IF(!is_valgrind(), float, float, ldouble, long double,     FLT_MAX)

/*
 * Cast from 'double'
 */

TEST_SUCCESS_FUNC(                   double, double, float,   float,       NEG_FLT_MAX)
TEST_SUCCESS_FUNC(                   double, double, float,   float,           FLT_MAX)
TEST_SUCCESS_FUNC_IF(!is_valgrind(), double, double, ldouble, long double, NEG_DBL_MAX)
TEST_SUCCESS_FUNC_IF(!is_valgrind(), double, double, ldouble, long double,     DBL_MAX)

TEST_E_ERRNO_FUNC(ERANGE, double, double, float, float, NEG_FLT_MAX_MUL_TWO)
TEST_E_ERRNO_FUNC(ERANGE, double, double, float, float,     FLT_MAX_MUL_TWO)

/*
 * Cast from 'long double'
 */

TEST_SUCCESS_FUNC_IF(!is_valgrind(), ldouble, long double, float,  float,  NEG_FLT_MAX)
TEST_SUCCESS_FUNC_IF(!is_valgrind(), ldouble, long double, float,  float,      FLT_MAX)
TEST_SUCCESS_FUNC_IF(!is_valgrind(), ldouble, long double, double, double, NEG_DBL_MAX)
TEST_SUCCESS_FUNC_IF(!is_valgrind(), ldouble, long double, double, double,     DBL_MAX)

TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ERANGE, ldouble, long double, float,  float,  NEG_FLT_MAX_MUL_TWO)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ERANGE, ldouble, long double, float,  float,      FLT_MAX_MUL_TWO)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ERANGE, ldouble, long double, double, double, NEG_DBL_MAX_MUL_TWO)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ERANGE, ldouble, long double, double, double,     DBL_MAX_MUL_TWO)

/*
 * Test functions
 */

static const struct test_func test[] = {
    /* Cast from '_Bool' */
    TEST_SUCCESS(_Bool, char, false),
    TEST_SUCCESS(_Bool, char, true),
    TEST_SUCCESS(_Bool, schar, false),
    TEST_SUCCESS(_Bool, schar, true),
    TEST_SUCCESS(_Bool, short, false),
    TEST_SUCCESS(_Bool, short, true),
    TEST_SUCCESS(_Bool, int, false),
    TEST_SUCCESS(_Bool, int, true),
    TEST_SUCCESS(_Bool, long, false),
    TEST_SUCCESS(_Bool, long, true),
    TEST_SUCCESS(_Bool, llong, false),
    TEST_SUCCESS(_Bool, llong, true),
    TEST_SUCCESS(_Bool, uchar, false),
    TEST_SUCCESS(_Bool, uchar, true),
    TEST_SUCCESS(_Bool, ushort, false),
    TEST_SUCCESS(_Bool, ushort, true),
    TEST_SUCCESS(_Bool, uint, false),
    TEST_SUCCESS(_Bool, uint, true),
    TEST_SUCCESS(_Bool, ulong, false),
    TEST_SUCCESS(_Bool, ulong, true),
    TEST_SUCCESS(_Bool, ullong, false),
    TEST_SUCCESS(_Bool, ullong, true),
    TEST_SUCCESS(_Bool, double, false),
    TEST_SUCCESS(_Bool, double, true),
    TEST_SUCCESS(_Bool, float, false),
    TEST_SUCCESS(_Bool, float, true),
    TEST_SUCCESS(_Bool, ldouble, false),
    TEST_SUCCESS(_Bool, ldouble, true),
    /* Cast from 'char' */
    TEST_SUCCESS(char, _Bool, 0),
    TEST_SUCCESS(char, _Bool, 1),
    TEST_SUCCESS(char, schar, 0),
    TEST_SUCCESS(char, schar, SCHAR_MAX),
    TEST_SUCCESS(char, short, CHAR_MIN),
    TEST_SUCCESS(char, short, CHAR_MAX),
    TEST_SUCCESS(char, int, CHAR_MIN),
    TEST_SUCCESS(char, int, CHAR_MAX),
    TEST_SUCCESS(char, long, CHAR_MIN),
    TEST_SUCCESS(char, long, CHAR_MAX),
    TEST_SUCCESS(char, llong, CHAR_MIN),
    TEST_SUCCESS(char, llong, CHAR_MAX),
    TEST_SUCCESS(char, uchar, 0),
    TEST_SUCCESS(char, uchar, CHAR_MAX),
    TEST_SUCCESS(char, ushort, 0),
    TEST_SUCCESS(char, ushort, CHAR_MAX),
    TEST_SUCCESS(char, uint, 0),
    TEST_SUCCESS(char, uint, CHAR_MAX),
    TEST_SUCCESS(char, ulong, 0),
    TEST_SUCCESS(char, ulong, CHAR_MAX),
    TEST_SUCCESS(char, ullong, 0),
    TEST_SUCCESS(char, ullong, CHAR_MAX),
    TEST_SUCCESS(char, double, CHAR_MIN),
    TEST_SUCCESS(char, double, CHAR_MAX),
    TEST_SUCCESS(char, float, CHAR_MIN),
    TEST_SUCCESS(char, float, CHAR_MAX),
    TEST_SUCCESS(char, ldouble, CHAR_MIN),
    TEST_SUCCESS(char, ldouble, CHAR_MAX),
    TEST_E_ERRNO(ERANGE, char, _Bool, NEG_ONE),
    TEST_E_ERRNO(ERANGE, char, _Bool, POS_TWO),
    /* Cast from 'signed char' */
    TEST_SUCCESS(schar, _Bool, 0),
    TEST_SUCCESS(schar, _Bool, 1),
    TEST_SUCCESS(schar, char, 0),
    TEST_SUCCESS(schar, char, SCHAR_MAX),
    TEST_SUCCESS(schar, short, SCHAR_MIN),
    TEST_SUCCESS(schar, short, SCHAR_MAX),
    TEST_SUCCESS(schar, int, SCHAR_MIN),
    TEST_SUCCESS(schar, int, SCHAR_MAX),
    TEST_SUCCESS(schar, long, SCHAR_MIN),
    TEST_SUCCESS(schar, long, SCHAR_MAX),
    TEST_SUCCESS(schar, llong, SCHAR_MIN),
    TEST_SUCCESS(schar, llong, SCHAR_MAX),
    TEST_SUCCESS(schar, uchar, 0),
    TEST_SUCCESS(schar, uchar, SCHAR_MAX),
    TEST_SUCCESS(schar, ushort, 0),
    TEST_SUCCESS(schar, ushort, SCHAR_MAX),
    TEST_SUCCESS(schar, uint, 0),
    TEST_SUCCESS(schar, uint, SCHAR_MAX),
    TEST_SUCCESS(schar, ulong, 0),
    TEST_SUCCESS(schar, ulong, SCHAR_MAX),
    TEST_SUCCESS(schar, ullong, 0),
    TEST_SUCCESS(schar, ullong, SCHAR_MAX),
    TEST_SUCCESS(schar, double, SCHAR_MIN),
    TEST_SUCCESS(schar, double, SCHAR_MAX),
    TEST_SUCCESS(schar, float, SCHAR_MIN),
    TEST_SUCCESS(schar, float, SCHAR_MAX),
    TEST_SUCCESS(schar, ldouble, SCHAR_MIN),
    TEST_SUCCESS(schar, ldouble, SCHAR_MAX),
    TEST_E_ERRNO(ERANGE, schar, _Bool, NEG_ONE),
    TEST_E_ERRNO(ERANGE, schar, _Bool, POS_TWO),
    /* Cast from 'short' */
    TEST_SUCCESS(short, _Bool, 0),
    TEST_SUCCESS(short, _Bool, 1),
    TEST_SUCCESS(short, char, CHAR_MIN),
    TEST_SUCCESS(short, char, CHAR_MAX),
    TEST_SUCCESS(short, schar, SCHAR_MIN),
    TEST_SUCCESS(short, schar, SCHAR_MAX),
    TEST_SUCCESS(short, int, SHRT_MIN),
    TEST_SUCCESS(short, int, SHRT_MAX),
    TEST_SUCCESS(short, long, SHRT_MIN),
    TEST_SUCCESS(short, long, SHRT_MAX),
    TEST_SUCCESS(short, llong, SHRT_MIN),
    TEST_SUCCESS(short, llong, SHRT_MAX),
    TEST_SUCCESS(short, uchar, 0),
    TEST_SUCCESS(short, uchar, CHAR_MAX),
    TEST_SUCCESS(short, ushort, 0),
    TEST_SUCCESS(short, ushort, SHRT_MAX),
    TEST_SUCCESS(short, uint, 0),
    TEST_SUCCESS(short, uint, SHRT_MAX),
    TEST_SUCCESS(short, ulong, 0),
    TEST_SUCCESS(short, ulong, SHRT_MAX),
    TEST_SUCCESS(short, ullong, 0),
    TEST_SUCCESS(short, ullong, SHRT_MAX),
    TEST_SUCCESS(short, double, SHRT_MIN),
    TEST_SUCCESS(short, double, SHRT_MAX),
    TEST_SUCCESS(short, float, SHRT_MIN),
    TEST_SUCCESS(short, float, SHRT_MAX),
    TEST_SUCCESS(short, ldouble, SHRT_MIN),
    TEST_SUCCESS(short, ldouble, SHRT_MAX),
    TEST_E_ERRNO(ERANGE, short, _Bool, NEG_ONE),
    TEST_E_ERRNO(ERANGE, short, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, short, schar, SCHAR_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, short, schar, SCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, short, uchar, UCHAR_MAX_ADD_ONE),
    /* Cast from 'int' */
    TEST_SUCCESS(int, _Bool, 0),
    TEST_SUCCESS(int, _Bool, 1),
    TEST_SUCCESS(int, char, CHAR_MIN),
    TEST_SUCCESS(int, char, CHAR_MAX),
    TEST_SUCCESS(int, schar, SCHAR_MIN),
    TEST_SUCCESS(int, schar, SCHAR_MAX),
    TEST_SUCCESS(int, short, SHRT_MIN),
    TEST_SUCCESS(int, short, SHRT_MAX),
    TEST_SUCCESS(int, long, INT_MIN),
    TEST_SUCCESS(int, long, INT_MAX),
    TEST_SUCCESS(int, llong, INT_MIN),
    TEST_SUCCESS(int, llong, INT_MAX),
    TEST_SUCCESS(int, uchar, 0),
    TEST_SUCCESS(int, uchar, CHAR_MAX),
    TEST_SUCCESS(int, ushort, 0),
    TEST_SUCCESS(int, ushort, SHRT_MAX),
    TEST_SUCCESS(int, uint, 0),
    TEST_SUCCESS(int, uint, INT_MAX),
    TEST_SUCCESS(int, ulong, 0),
    TEST_SUCCESS(int, ulong, INT_MAX),
    TEST_SUCCESS(int, ullong, 0),
    TEST_SUCCESS(int, ullong, INT_MAX),
    TEST_SUCCESS(int, double, INT_MIN),
    TEST_SUCCESS(int, double, INT_MAX),
    TEST_SUCCESS(int, float, INT_MIN),
    TEST_SUCCESS(int, float, INT_MAX),
    TEST_SUCCESS(int, ldouble, INT_MIN),
    TEST_SUCCESS(int, ldouble, INT_MAX),
    TEST_E_ERRNO(ERANGE, int, _Bool, NEG_ONE),
    TEST_E_ERRNO(ERANGE, int, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, int, schar, SCHAR_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, int, schar, SCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, int, short, SHRT_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, int, short, SHRT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, int, uchar, UCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, int, ushort, USHRT_MAX_ADD_ONE),
    /* Cast from 'long' */
    TEST_SUCCESS(long, _Bool, 0),
    TEST_SUCCESS(long, _Bool, 1),
    TEST_SUCCESS(long, char, CHAR_MIN),
    TEST_SUCCESS(long, char, CHAR_MAX),
    TEST_SUCCESS(long, schar, SCHAR_MIN),
    TEST_SUCCESS(long, schar, SCHAR_MAX),
    TEST_SUCCESS(long, short, SHRT_MIN),
    TEST_SUCCESS(long, short, SHRT_MAX),
    TEST_SUCCESS(long, int, INT_MIN),
    TEST_SUCCESS(long, int, INT_MAX),
    TEST_SUCCESS(long, llong, LONG_MIN),
    TEST_SUCCESS(long, llong, LONG_MAX),
    TEST_SUCCESS(long, uchar, 0),
    TEST_SUCCESS(long, uchar, CHAR_MAX),
    TEST_SUCCESS(long, ushort, 0),
    TEST_SUCCESS(long, ushort, SHRT_MAX),
    TEST_SUCCESS(long, uint, 0),
    TEST_SUCCESS(long, uint, INT_MAX),
    TEST_SUCCESS(long, ulong, 0),
    TEST_SUCCESS(long, ulong, LONG_MAX),
    TEST_SUCCESS(long, ullong, 0),
    TEST_SUCCESS(long, ullong, LONG_MAX),
    TEST_SUCCESS(long, double, LONG_MIN),
    TEST_SUCCESS(long, double, LONG_MAX),
    TEST_SUCCESS(long, float, LONG_MIN),
    TEST_SUCCESS(long, float, LONG_MAX),
    TEST_SUCCESS(long, ldouble, LONG_MIN),
    TEST_SUCCESS(long, ldouble, LONG_MAX),
    TEST_E_ERRNO(ERANGE, long, _Bool, NEG_ONE),
    TEST_E_ERRNO(ERANGE, long, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, long, schar, SCHAR_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, long, schar, SCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, long, short, SHRT_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, long, short, SHRT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, long, int, INT_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, long, int, INT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, long, uchar, UCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, long, ushort, USHRT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, long, uint, UINT_MAX_ADD_ONE),
    /* Cast from 'long long' */
    TEST_SUCCESS(llong, _Bool, 0),
    TEST_SUCCESS(llong, _Bool, 1),
    TEST_SUCCESS(llong, char, CHAR_MIN),
    TEST_SUCCESS(llong, char, CHAR_MAX),
    TEST_SUCCESS(llong, schar, SCHAR_MIN),
    TEST_SUCCESS(llong, schar, SCHAR_MAX),
    TEST_SUCCESS(llong, short, SHRT_MIN),
    TEST_SUCCESS(llong, short, SHRT_MAX),
    TEST_SUCCESS(llong, int, INT_MIN),
    TEST_SUCCESS(llong, int, INT_MAX),
    TEST_SUCCESS(llong, long, LONG_MIN),
    TEST_SUCCESS(llong, long, LONG_MAX),
    TEST_SUCCESS(llong, uchar, 0),
    TEST_SUCCESS(llong, uchar, CHAR_MAX),
    TEST_SUCCESS(llong, ushort, 0),
    TEST_SUCCESS(llong, ushort, SHRT_MAX),
    TEST_SUCCESS(llong, uint, 0),
    TEST_SUCCESS(llong, uint, UINT_MAX),
    TEST_SUCCESS(llong, ulong, 0),
    TEST_SUCCESS(llong, ulong, LONG_MAX),
    TEST_SUCCESS(llong, ullong, 0),
    TEST_SUCCESS(llong, ullong, LLONG_MAX),
    TEST_SUCCESS(llong, double, LLONG_MIN),
    TEST_SUCCESS(llong, double, LLONG_MAX),
    TEST_SUCCESS(llong, float, LLONG_MIN),
    TEST_SUCCESS(llong, float, LLONG_MAX),
    TEST_SUCCESS(llong, ldouble, LLONG_MIN),
    TEST_SUCCESS(llong, ldouble, LLONG_MAX),
    TEST_E_ERRNO(ERANGE, llong, _Bool, NEG_ONE),
    TEST_E_ERRNO(ERANGE, llong, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, llong, schar, SCHAR_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, llong, schar, SCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, llong, short, SHRT_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, llong, short, SHRT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, llong, int, INT_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, llong, int, INT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, llong, long, LONG_MIN_SUB_ONE),
    TEST_E_ERRNO(ERANGE, llong, long, LONG_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, llong, uchar, UCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, llong, ushort, USHRT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, llong, uint, UINT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, llong, ulong, ULONG_MAX_ADD_ONE),
    /* Cast from 'unsigned char' */
    TEST_SUCCESS(uchar, _Bool, 0),
    TEST_SUCCESS(uchar, _Bool, 1),
    TEST_SUCCESS(uchar, char, 0),
    TEST_SUCCESS(uchar, char, CHAR_MAX),
    TEST_SUCCESS(uchar, schar, 0),
    TEST_SUCCESS(uchar, schar, SCHAR_MAX),
    TEST_SUCCESS(uchar, short, 0),
    TEST_SUCCESS(uchar, short, UCHAR_MAX),
    TEST_SUCCESS(uchar, int, 0),
    TEST_SUCCESS(uchar, int, UCHAR_MAX),
    TEST_SUCCESS(uchar, long, 0),
    TEST_SUCCESS(uchar, long, UCHAR_MAX),
    TEST_SUCCESS(uchar, llong, 0),
    TEST_SUCCESS(uchar, llong, UCHAR_MAX),
    TEST_SUCCESS(uchar, ushort, 0),
    TEST_SUCCESS(uchar, ushort, UCHAR_MAX),
    TEST_SUCCESS(uchar, uint, 0),
    TEST_SUCCESS(uchar, uint, UCHAR_MAX),
    TEST_SUCCESS(uchar, ulong, 0),
    TEST_SUCCESS(uchar, ulong, UCHAR_MAX),
    TEST_SUCCESS(uchar, ullong, 0),
    TEST_SUCCESS(uchar, ullong, UCHAR_MAX),
    TEST_SUCCESS(uchar, double, 0),
    TEST_SUCCESS(uchar, double, UCHAR_MAX),
    TEST_SUCCESS(uchar, float, 0),
    TEST_SUCCESS(uchar, float, UCHAR_MAX),
    TEST_SUCCESS(uchar, ldouble, 0),
    TEST_SUCCESS(uchar, ldouble, UCHAR_MAX),
    TEST_E_ERRNO(ERANGE, uchar, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, uchar, schar, SCHAR_MAX_ADD_ONE_U),
    /* Cast from 'unsigned short' */
    TEST_SUCCESS(ushort, _Bool, 0),
    TEST_SUCCESS(ushort, _Bool, 1),
    TEST_SUCCESS(ushort, char, 0),
    TEST_SUCCESS(ushort, char, CHAR_MAX),
    TEST_SUCCESS(ushort, schar, 0),
    TEST_SUCCESS(ushort, schar, SCHAR_MAX),
    TEST_SUCCESS(ushort, short, 0),
    TEST_SUCCESS(ushort, short, SHRT_MAX),
    TEST_SUCCESS(ushort, int, 0),
    TEST_SUCCESS(ushort, int, USHRT_MAX),
    TEST_SUCCESS(ushort, long, 0),
    TEST_SUCCESS(ushort, long, USHRT_MAX),
    TEST_SUCCESS(ushort, llong, 0),
    TEST_SUCCESS(ushort, llong, USHRT_MAX),
    TEST_SUCCESS(ushort, uchar, 0),
    TEST_SUCCESS(ushort, uchar, UCHAR_MAX),
    TEST_SUCCESS(ushort, uint, 0),
    TEST_SUCCESS(ushort, uint, USHRT_MAX),
    TEST_SUCCESS(ushort, ulong, 0),
    TEST_SUCCESS(ushort, ulong, USHRT_MAX),
    TEST_SUCCESS(ushort, ullong, 0),
    TEST_SUCCESS(ushort, ullong, USHRT_MAX),
    TEST_SUCCESS(ushort, double, 0),
    TEST_SUCCESS(ushort, double, USHRT_MAX),
    TEST_SUCCESS(ushort, float, 0),
    TEST_SUCCESS(ushort, float, USHRT_MAX),
    TEST_SUCCESS(ushort, ldouble, 0),
    TEST_SUCCESS(ushort, ldouble, USHRT_MAX),
    TEST_E_ERRNO(ERANGE, ushort, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, ushort, schar, SCHAR_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ushort, short, SHRT_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ushort, uchar, UCHAR_MAX_ADD_ONE),
    /* Cast from 'unsigned int' */
    TEST_SUCCESS(uint, _Bool, 0),
    TEST_SUCCESS(uint, _Bool, 1),
    TEST_SUCCESS(uint, char, 0),
    TEST_SUCCESS(uint, char, CHAR_MAX),
    TEST_SUCCESS(uint, schar, 0),
    TEST_SUCCESS(uint, schar, SCHAR_MAX),
    TEST_SUCCESS(uint, short, 0),
    TEST_SUCCESS(uint, short, SHRT_MAX),
    TEST_SUCCESS(uint, int, 0),
    TEST_SUCCESS(uint, int, INT_MAX),
    TEST_SUCCESS(uint, long, 0),
    TEST_SUCCESS(uint, long, UINT_MAX),
    TEST_SUCCESS(uint, llong, 0),
    TEST_SUCCESS(uint, llong, UINT_MAX),
    TEST_SUCCESS(uint, uchar, 0),
    TEST_SUCCESS(uint, uchar, UCHAR_MAX),
    TEST_SUCCESS(uint, ushort, 0),
    TEST_SUCCESS(uint, ushort, USHRT_MAX),
    TEST_SUCCESS(uint, ulong, 0),
    TEST_SUCCESS(uint, ulong, UINT_MAX),
    TEST_SUCCESS(uint, ullong, 0),
    TEST_SUCCESS(uint, ullong, UINT_MAX),
    TEST_SUCCESS(uint, double, 0),
    TEST_SUCCESS(uint, double, UINT_MAX),
    TEST_SUCCESS(uint, float, 0),
    TEST_SUCCESS(uint, float, UINT_MAX),
    TEST_SUCCESS(uint, ldouble, 0),
    TEST_SUCCESS(uint, ldouble, UINT_MAX),
    TEST_E_ERRNO(ERANGE, uint, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, uint, schar, SCHAR_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, uint, short, SHRT_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, uint, int, INT_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, uint, uchar, UCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, uint, ushort, USHRT_MAX_ADD_ONE),
    /* Cast from 'unsigned long' */
    TEST_SUCCESS(ulong, _Bool, 0),
    TEST_SUCCESS(ulong, _Bool, 1),
    TEST_SUCCESS(ulong, char, 0),
    TEST_SUCCESS(ulong, char, CHAR_MAX),
    TEST_SUCCESS(ulong, schar, 0),
    TEST_SUCCESS(ulong, schar, SCHAR_MAX),
    TEST_SUCCESS(ulong, short, 0),
    TEST_SUCCESS(ulong, short, SHRT_MAX),
    TEST_SUCCESS(ulong, int, 0),
    TEST_SUCCESS(ulong, int, INT_MAX),
    TEST_SUCCESS(ulong, long, 0),
    TEST_SUCCESS(ulong, long, LONG_MAX),
    TEST_SUCCESS(ulong, llong, 0),
    TEST_SUCCESS(ulong, llong, LLONG_MAX),
    TEST_SUCCESS(ulong, uchar, 0),
    TEST_SUCCESS(ulong, uchar, UCHAR_MAX),
    TEST_SUCCESS(ulong, ushort, 0),
    TEST_SUCCESS(ulong, ushort, USHRT_MAX),
    TEST_SUCCESS(ulong, uint, 0),
    TEST_SUCCESS(ulong, uint, UINT_MAX),
    TEST_SUCCESS(ulong, ullong, 0),
    TEST_SUCCESS(ulong, ullong, ULONG_MAX),
    TEST_SUCCESS(ulong, double, 0),
    TEST_SUCCESS(ulong, double, ULONG_MAX),
    TEST_SUCCESS(ulong, float, 0),
    TEST_SUCCESS(ulong, float, ULONG_MAX),
    TEST_SUCCESS(ulong, ldouble, 0),
    TEST_SUCCESS(ulong, ldouble, ULONG_MAX),
    TEST_E_ERRNO(ERANGE, ulong, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, ulong, schar, SCHAR_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ulong, short, SHRT_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ulong, int, INT_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ulong, long, LONG_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ulong, uchar, UCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, ulong, ushort, USHRT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, ulong, uint, UINT_MAX_ADD_ONE),
    /* Cast from 'unsigned long long' */
    TEST_SUCCESS(ullong, _Bool, 0),
    TEST_SUCCESS(ullong, _Bool, 1),
    TEST_SUCCESS(ullong, char, 0),
    TEST_SUCCESS(ullong, char, CHAR_MAX),
    TEST_SUCCESS(ullong, schar, 0),
    TEST_SUCCESS(ullong, schar, SCHAR_MAX),
    TEST_SUCCESS(ullong, short, 0),
    TEST_SUCCESS(ullong, short, SHRT_MAX),
    TEST_SUCCESS(ullong, int, 0),
    TEST_SUCCESS(ullong, int, INT_MAX),
    TEST_SUCCESS(ullong, long, 0),
    TEST_SUCCESS(ullong, long, LONG_MAX),
    TEST_SUCCESS(ullong, llong, 0),
    TEST_SUCCESS(ullong, llong, LLONG_MAX),
    TEST_SUCCESS(ullong, uchar, 0),
    TEST_SUCCESS(ullong, uchar, UCHAR_MAX),
    TEST_SUCCESS(ullong, ushort, 0),
    TEST_SUCCESS(ullong, ushort, USHRT_MAX),
    TEST_SUCCESS(ullong, uint, 0),
    TEST_SUCCESS(ullong, uint, UINT_MAX),
    TEST_SUCCESS(ullong, ulong, 0),
    TEST_SUCCESS(ullong, ulong, ULONG_MAX),
    TEST_SUCCESS(ullong, double, 0),
    TEST_SUCCESS(ullong, double, ULLONG_MAX),
    TEST_SUCCESS(ullong, float, 0),
    TEST_SUCCESS(ullong, float, ULLONG_MAX),
    TEST_SUCCESS(ullong, ldouble, 0),
    TEST_SUCCESS(ullong, ldouble, ULLONG_MAX),
    TEST_E_ERRNO(ERANGE, ullong, _Bool, POS_TWO),
    TEST_E_ERRNO(ERANGE, ullong, schar, SCHAR_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ullong, short, SHRT_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ullong, int, INT_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ullong, long, LONG_MAX_ADD_ONE_U),
    TEST_E_ERRNO(ERANGE, ullong, uchar, UCHAR_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, ullong, ushort, USHRT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, ullong, uint, UINT_MAX_ADD_ONE),
    TEST_E_ERRNO(ERANGE, ullong, ulong, ULONG_MAX_ADD_ONE),
    /* Cast from 'float' */
    TEST_SUCCESS(float, double,  NEG_FLT_MAX),
    TEST_SUCCESS(float, double,      FLT_MAX),
    TEST_SUCCESS(float, ldouble, NEG_FLT_MAX),
    TEST_SUCCESS(float, ldouble,     FLT_MAX),
    /* Cast from 'double' */
    TEST_SUCCESS(double, float,   NEG_FLT_MAX),
    TEST_SUCCESS(double, float,       FLT_MAX),
    TEST_SUCCESS(double, ldouble, NEG_DBL_MAX),
    TEST_SUCCESS(double, ldouble,     DBL_MAX),
    TEST_E_ERRNO(ERANGE, double, float, NEG_FLT_MAX_MUL_TWO),
    TEST_E_ERRNO(ERANGE, double, float,     FLT_MAX_MUL_TWO),
    /* Cast from 'long double' */
    TEST_SUCCESS(ldouble, float,  NEG_FLT_MAX),
    TEST_SUCCESS(ldouble, float,      FLT_MAX),
    TEST_SUCCESS(ldouble, double, NEG_DBL_MAX),
    TEST_SUCCESS(ldouble, double,     DBL_MAX),
    TEST_E_ERRNO(ERANGE, ldouble, float,  NEG_FLT_MAX_MUL_TWO),
    TEST_E_ERRNO(ERANGE, ldouble, float,      FLT_MAX_MUL_TWO),
    TEST_E_ERRNO(ERANGE, ldouble, double, NEG_DBL_MAX_MUL_TWO),
    TEST_E_ERRNO(ERANGE, ldouble, double,     DBL_MAX_MUL_TWO)
};

/*
 * Entry point
 */

#include "opts.h"
#include "pubapi.h"

int
main(int argc, char* argv[])
{
    return pubapi_main(argc, argv, PARSE_OPTS_STRING(), test, arraylen(test));
}
