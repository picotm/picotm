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

#include "picotm/picotm.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-arithmetic-ctypes.h"
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

#define EPSILON (0.0001)

static bool
eqc(char lhs, char rhs)
{
    return lhs == rhs;
}

static bool
eqll(long long lhs, long long rhs)
{
    return lhs == rhs;
}

static bool
equll(unsigned long long lhs, unsigned long long rhs)
{
    return lhs == rhs;
}

static bool
feql(long double lhs, long double rhs)
{
    const long double diff = lhs - rhs;

    return (-EPSILON < diff) && (diff < EPSILON);
}

#define __eq(_expr)                         \
    _Generic((_expr),                       \
              _Bool: eqll,                  \
              char: eqc,                    \
              signed char: eqll,            \
              short: eqll,                  \
              int: eqll,                    \
              long: eqll,                   \
              long long: eqll,              \
              unsigned char: equll,         \
              unsigned short: equll,        \
              unsigned int: equll,          \
              unsigned long: equll,         \
              unsigned long long: equll,    \
              float: feql,                  \
              double: feql,                 \
              long double: feql)

#define eq(_lhs, _rhs)  __eq(_lhs)(_lhs, _rhs)

/*
 * Test interfaces of <picotm/picotm-cast.h>
 */

#define __TEST_SYMBOL(_func)    \
    test_ ## _func

#define __TEST_SKIP_MSG(_funcstr)  \
    "Skipping test for " _funcstr ".\n"

/*
 * Test for success
 */

#define __TEST_SUCCESS_STRSYM(_func, _name, _value)                         \
    __TEST_SYMBOL(success_ ## _func ## _ ## _name ## _tx_ ## _value ## _str)

#define __TEST_SUCCESS_SYMBOL(_func, _name, _value)                     \
    __TEST_SYMBOL(success_ ## _func ## _ ## _name ## _tx_ ## _value)

#define __TEST_SUCCESS_FUNC(_sym, _cond, _name, _type, _value, _func, ...)  \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        if (!(_cond)) {                                                     \
            tap_info(__TEST_SKIP_MSG(#_func "_" #_name "_tx()"));           \
            return;                                                         \
        }                                                                   \
        _type value = (_value);                                             \
        picotm_begin                                                        \
            _type result = _func ## _ ## _name ## _tx(__VA_ARGS__);         \
            if (!eq(result, value)) {                                       \
                tap_error("incorrect result for " #_func " operation\n");   \
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;       \
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);  \
                picotm_error_mark_as_non_recoverable(&error);               \
                picotm_recover_from_error(&error);                          \
            }                                                               \
        picotm_commit                                                       \
            abort_transaction_on_error(__func__);                           \
        picotm_end                                                          \
    }

#define __TEST_SUCCESS_STRBUF(_funcstr, _namestr, _argstr, _valuestr)       \
    "Testing for " _funcstr "_" _namestr "_tx(" _argstr ") == " _valuestr;

#define TEST_SUCCESS_FUNC_IF(_cond, _name, _type, _i, _value, _func, ...)   \
    static const char                                                       \
        __TEST_SUCCESS_STRSYM(_func, _name, _i ## _ ## _value)[] =          \
        __TEST_SUCCESS_STRBUF(#_func, #_name, #__VA_ARGS__, #_value);       \
    __TEST_SUCCESS_FUNC(                                                    \
        __TEST_SUCCESS_SYMBOL(_func, _name, _i ## _ ## _value),             \
        _cond, _name, _type, _value, _func, __VA_ARGS__)

#define TEST_SUCCESS_FUNC(_name, _type, _i, _value, _func, ...)         \
    static const char                                                   \
        __TEST_SUCCESS_STRSYM(_func, _name, _i ## _ ## _value)[] =      \
        __TEST_SUCCESS_STRBUF(#_func, #_name, #__VA_ARGS__, #_value);   \
    __TEST_SUCCESS_FUNC(                                                \
        __TEST_SUCCESS_SYMBOL(_func, _name, _i ## _ ## _value),         \
        true, _name, _type, _value, _func, __VA_ARGS__)

#define TEST_SUCCESS(_i, _name, _value, _func)                  \
    {__TEST_SUCCESS_STRSYM(_func, _name, _i ## _ ## _value),         \
        __TEST_SUCCESS_SYMBOL(_func, _name, _i ## _ ## _value), \
        NULL, NULL}

/*
 * Test for error
 */

#define __TEST_ERROR_STRSYM(_error, _func, _name, _value)                   \
    __TEST_SYMBOL(                                                          \
        e_ ## _error ## _ ## _func ## _ ## _name ## _tx_ ## _value ## _str)

#define __TEST_ERROR_SYMBOL(_error, _func, _name, _value)           \
    __TEST_SYMBOL(                                                  \
        e_ ## _error ## _ ## _func ## _ ## _name ## _tx_ ## _value)

/* Test for errno code */

#define __TEST_E_ERRNO_FUNC(_sym, _cond, _errno, _name, _type, _func, ...)  \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        if (!(_cond)) {                                                     \
            tap_info(__TEST_SKIP_MSG(#_func "_" #_name "_tx()"));           \
            return;                                                         \
        }                                                                   \
        bool error_detected = false;                                        \
        errno = 0;                                                          \
        picotm_begin                                                        \
            _type result UNUSED =                                           \
                _func ## _ ## _name ## _tx(__VA_ARGS__);                    \
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

#define __TEST_E_ERRNO_STRBUF(_errnostr, _funcstr, _namestr, _argstr) \
    "Testing for errno " _errnostr " from " _funcstr "_" _namestr "_tx(" _argstr ")";

#define TEST_E_ERRNO_FUNC_IF(_cond, _name, _type, _i, _errno, _func, ... )  \
    static const char                                                       \
        __TEST_ERROR_STRSYM(errno_ ## _errno, _func, _name, _i)[] =       \
        __TEST_E_ERRNO_STRBUF(#_errno, #_func, #_name, #__VA_ARGS__);       \
    __TEST_E_ERRNO_FUNC(                                                    \
        __TEST_ERROR_SYMBOL(errno_ ## _errno, _func, _name, _i),            \
        _cond, _errno, _name, _type, _func, __VA_ARGS__)

#define TEST_E_ERRNO_FUNC(_name, _type, _i, _errno, _func, ... )        \
    static const char                                                   \
        __TEST_ERROR_STRSYM(errno_ ## _errno, _func, _name, _i)[] =   \
        __TEST_E_ERRNO_STRBUF(#_errno, #_func, #_name, #__VA_ARGS__);   \
    __TEST_E_ERRNO_FUNC(                                                \
        __TEST_ERROR_SYMBOL(errno_ ## _errno, _func, _name, _i),        \
        true, _errno, _name, _type, _func, __VA_ARGS__)

#define TEST_E_ERRNO(_i, _name, _errno, _func)                      \
    {__TEST_ERROR_STRSYM(errno_ ## _errno, _func, _name, _i),       \
        __TEST_ERROR_SYMBOL(errno_ ## _errno, _func, _name, _i),    \
        NULL, NULL}

/*
 * Test constants
 */

#define NEG_ONE         (-1)
#define NEG_TWO         (-2)

#define NEG_SCHAR_MAX   (-SCHAR_MAX)
#define NEG_SHRT_MAX    (-SHRT_MAX)
#define NEG_INT_MAX     (-INT_MAX)
#define NEG_LONG_MAX    (-LONG_MAX)
#define NEG_LLONG_MAX   (-LLONG_MAX)

#define POS_FLT_EXT     (FLT_MAX)
#define NEG_FLT_EXT     (-POS_FLT_EXT)

#define POS_DBL_EXT     (DBL_MAX)
#define NEG_DBL_EXT     (-POS_DBL_EXT)

#define POS_LDBL_EXT    (LDBL_MAX)
#define NEG_LDBL_EXT    (-POS_LDBL_EXT)

/*
 * '_Bool'
 */

TEST_SUCCESS_FUNC(_Bool, _Bool, 0, false,  add, false, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, 1, true,   add, false, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, 2, true,   add, true, false)

TEST_SUCCESS_FUNC(_Bool, _Bool, 0, false, sub, false, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, 1, false, sub, true, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, 2, true,  sub, true, false)

TEST_SUCCESS_FUNC(_Bool, _Bool, 0, false, mul, false, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, 1, false, mul, false, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, 2, false, mul, true, false)
TEST_SUCCESS_FUNC(_Bool, _Bool, 3, true,  mul, true, true)

TEST_SUCCESS_FUNC(_Bool, _Bool, 0, false, div, false, true)
TEST_SUCCESS_FUNC(_Bool, _Bool, 1, true,  div, true, true)

TEST_E_ERRNO_FUNC(_Bool, _Bool, 0, ERANGE, add, true, true)
TEST_E_ERRNO_FUNC(_Bool, _Bool, 1, ERANGE, sub, false, true)

/*
 * 'char'
 */

TEST_SUCCESS_FUNC(char, char, 0, 0,        add, 0, 0)
TEST_SUCCESS_FUNC(char, char, 1, CHAR_MIN, add, 0, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, 2, CHAR_MIN, add, CHAR_MIN, 0)
TEST_SUCCESS_FUNC(char, char, 3, CHAR_MAX, add, 0, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, 4, CHAR_MAX, add, CHAR_MAX, 0)
TEST_SUCCESS_FUNC(char, char, 5, CHAR_MAX, add, 1, CHAR_MAX - 1)
TEST_SUCCESS_FUNC(char, char, 6, CHAR_MAX, add, CHAR_MAX - 1, 1)

TEST_SUCCESS_FUNC(char, char, 0, 0,        sub, 0, 0)
TEST_SUCCESS_FUNC(char, char, 1, CHAR_MIN, sub, CHAR_MIN, 0)
TEST_SUCCESS_FUNC(char, char, 2, CHAR_MIN, sub, CHAR_MIN + 1, 1)
TEST_SUCCESS_FUNC(char, char, 3, CHAR_MAX, sub, CHAR_MAX, 0)

TEST_SUCCESS_FUNC(char, char, 0, 0,        mul, 0, 0)
TEST_SUCCESS_FUNC(char, char, 1, 1,        mul, 1, 1)
TEST_SUCCESS_FUNC(char, char, 2, 0,        mul, CHAR_MIN, 0)
TEST_SUCCESS_FUNC(char, char, 3, 0,        mul, 0, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, 4, CHAR_MIN, mul, CHAR_MIN, 1)
TEST_SUCCESS_FUNC(char, char, 5, CHAR_MIN, mul, 1, CHAR_MIN)
TEST_SUCCESS_FUNC(char, char, 6, 0,        mul, CHAR_MAX, 0)
TEST_SUCCESS_FUNC(char, char, 7, 0,        mul, 0, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, 8, CHAR_MAX, mul, CHAR_MAX, 1)
TEST_SUCCESS_FUNC(char, char, 9, CHAR_MAX, mul, 1, CHAR_MAX)

TEST_SUCCESS_FUNC(char, char, 0, 0,        div, 0, 1)
TEST_SUCCESS_FUNC(char, char, 1, 1,        div, 1, 1)
TEST_SUCCESS_FUNC(char, char, 2, 1,        div, CHAR_MAX, CHAR_MAX)
TEST_SUCCESS_FUNC(char, char, 3, CHAR_MAX, div, CHAR_MAX, 1)

TEST_E_ERRNO_FUNC(char, char, 0, ERANGE, add, CHAR_MAX, 1)
TEST_E_ERRNO_FUNC(char, char, 1, ERANGE, sub, CHAR_MIN, 1)
TEST_E_ERRNO_FUNC(char, char, 2, ERANGE, mul, CHAR_MAX, 2)
TEST_E_ERRNO_FUNC(char, char, 3, EDOM,   div, 0, 0)

/*
 * 'signed char'
 */

TEST_SUCCESS_FUNC(schar, signed char,  0, 0,         add, 0, 0)
TEST_SUCCESS_FUNC(schar, signed char,  1, 0,         add, -SCHAR_MAX, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char,  2, 0,         add, SCHAR_MAX, -SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char,  3, SCHAR_MIN, add, 0, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char,  4, SCHAR_MIN, add, SCHAR_MIN, 0)
TEST_SUCCESS_FUNC(schar, signed char,  5, SCHAR_MIN, add, -1, SCHAR_MIN + 1)
TEST_SUCCESS_FUNC(schar, signed char,  6, SCHAR_MIN, add, SCHAR_MIN + 1, -1)
TEST_SUCCESS_FUNC(schar, signed char,  7, SCHAR_MAX, add, 0, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char,  9, SCHAR_MAX, add, SCHAR_MAX, 0)
TEST_SUCCESS_FUNC(schar, signed char,  8, SCHAR_MAX, add, 1, SCHAR_MAX - 1)
TEST_SUCCESS_FUNC(schar, signed char, 10, SCHAR_MAX, add, SCHAR_MAX - 1, 1)

TEST_SUCCESS_FUNC(schar, signed char, 0, 0,         sub, 0, 0)
TEST_SUCCESS_FUNC(schar, signed char, 1, 0,         sub, SCHAR_MAX, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, 2, 0,         sub, -SCHAR_MAX, -SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, 3, SCHAR_MIN, sub, SCHAR_MIN, 0)
TEST_SUCCESS_FUNC(schar, signed char, 4, SCHAR_MIN, sub, SCHAR_MIN + 1, 1)
TEST_SUCCESS_FUNC(schar, signed char, 5, SCHAR_MAX, sub, SCHAR_MAX, 0)
TEST_SUCCESS_FUNC(schar, signed char, 6, SCHAR_MAX, sub, SCHAR_MAX - 1, -1)

TEST_SUCCESS_FUNC(schar, signed char,  0, 0,             mul, 0, 0)
TEST_SUCCESS_FUNC(schar, signed char,  1, 1,             mul, 1, 1)
TEST_SUCCESS_FUNC(schar, signed char,  2, 0,             mul, SCHAR_MIN, 0)
TEST_SUCCESS_FUNC(schar, signed char,  3, 0,             mul, 0, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char,  4, SCHAR_MIN,     mul, SCHAR_MIN, 1)
TEST_SUCCESS_FUNC(schar, signed char,  5, SCHAR_MIN,     mul, 1, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char,  6, 0,             mul, SCHAR_MAX, 0)
TEST_SUCCESS_FUNC(schar, signed char,  7, 0,             mul, 0, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char,  8, SCHAR_MAX,     mul, SCHAR_MAX, 1)
TEST_SUCCESS_FUNC(schar, signed char,  9, SCHAR_MAX,     mul, 1, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, 10, SCHAR_MAX,     mul, -SCHAR_MAX, -1)
TEST_SUCCESS_FUNC(schar, signed char, 11, SCHAR_MAX,     mul, -1, -SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, 12, NEG_SCHAR_MAX, mul, SCHAR_MAX, -1)
TEST_SUCCESS_FUNC(schar, signed char, 13, NEG_SCHAR_MAX, mul, -1, SCHAR_MAX)

TEST_SUCCESS_FUNC(schar, signed char, 0, 0,         div, 0, 1)
TEST_SUCCESS_FUNC(schar, signed char, 1, 1,         div, 1, 1)
TEST_SUCCESS_FUNC(schar, signed char, 2, 1,         div, SCHAR_MIN, SCHAR_MIN)
TEST_SUCCESS_FUNC(schar, signed char, 3, SCHAR_MIN, div, SCHAR_MIN, 1)
TEST_SUCCESS_FUNC(schar, signed char, 4, 1,         div, SCHAR_MAX, SCHAR_MAX)
TEST_SUCCESS_FUNC(schar, signed char, 5, SCHAR_MAX, div, SCHAR_MAX, 1)

TEST_E_ERRNO_FUNC(schar, signed char, 0, ERANGE, add, SCHAR_MAX, 1)
TEST_E_ERRNO_FUNC(schar, signed char, 1, ERANGE, sub, SCHAR_MIN, 1)
TEST_E_ERRNO_FUNC(schar, signed char, 2, ERANGE, mul, SCHAR_MAX, 2)
TEST_E_ERRNO_FUNC(schar, signed char, 3, ERANGE, mul, SCHAR_MIN, 2)
TEST_E_ERRNO_FUNC(schar, signed char, 4, ERANGE, mul, SCHAR_MIN, -1)
TEST_E_ERRNO_FUNC(schar, signed char, 5, EDOM,   div, 0, 0)
TEST_E_ERRNO_FUNC(schar, signed char, 6, ERANGE, div, SCHAR_MIN, -1)

/*
 * 'short'
 */

TEST_SUCCESS_FUNC(short, short,  0, 0,        add, 0, 0)
TEST_SUCCESS_FUNC(short, short,  1, 0,        add, -SHRT_MAX, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short,  2, 0,        add, SHRT_MAX, -SHRT_MAX)
TEST_SUCCESS_FUNC(short, short,  3, SHRT_MIN, add, 0, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short,  4, SHRT_MIN, add, SHRT_MIN, 0)
TEST_SUCCESS_FUNC(short, short,  5, SHRT_MIN, add, -1, SHRT_MIN + 1)
TEST_SUCCESS_FUNC(short, short,  6, SHRT_MIN, add, SHRT_MIN + 1, -1)
TEST_SUCCESS_FUNC(short, short,  7, SHRT_MAX, add, 0, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short,  8, SHRT_MAX, add, SHRT_MAX, 0)
TEST_SUCCESS_FUNC(short, short,  9, SHRT_MAX, add, 1, SHRT_MAX - 1)
TEST_SUCCESS_FUNC(short, short, 10, SHRT_MAX, add, SHRT_MAX - 1, 1)

TEST_SUCCESS_FUNC(short, short, 0, 0,        sub, 0, 0)
TEST_SUCCESS_FUNC(short, short, 1, 0,        sub, SHRT_MAX, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, 2, 0,        sub, -SHRT_MAX, -SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, 3, SHRT_MIN, sub, SHRT_MIN, 0)
TEST_SUCCESS_FUNC(short, short, 4, SHRT_MIN, sub, SHRT_MIN + 1, 1)
TEST_SUCCESS_FUNC(short, short, 5, SHRT_MAX, sub, SHRT_MAX, 0)
TEST_SUCCESS_FUNC(short, short, 6, SHRT_MAX, sub, SHRT_MAX - 1, -1)

TEST_SUCCESS_FUNC(short, short,  0, 0,            mul, 0, 0)
TEST_SUCCESS_FUNC(short, short,  1, 1,            mul, 1, 1)
TEST_SUCCESS_FUNC(short, short,  2, 0,            mul, SHRT_MIN, 0)
TEST_SUCCESS_FUNC(short, short,  3, 0,            mul, 0, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short,  4, SHRT_MIN,     mul, SHRT_MIN, 1)
TEST_SUCCESS_FUNC(short, short,  5, SHRT_MIN,     mul, 1, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short,  6, 0,            mul, SHRT_MAX, 0)
TEST_SUCCESS_FUNC(short, short,  7, 0,            mul, 0, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short,  8, SHRT_MAX,     mul, SHRT_MAX, 1)
TEST_SUCCESS_FUNC(short, short,  9, SHRT_MAX,     mul, 1, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, 10, SHRT_MAX,     mul, -SHRT_MAX, -1)
TEST_SUCCESS_FUNC(short, short, 11, SHRT_MAX,     mul, -1, -SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, 12, NEG_SHRT_MAX, mul, SHRT_MAX, -1)
TEST_SUCCESS_FUNC(short, short, 13, NEG_SHRT_MAX, mul, -1, SHRT_MAX)

TEST_SUCCESS_FUNC(short, short, 0, 0,        div, 0, 1)
TEST_SUCCESS_FUNC(short, short, 1, 1,        div, 1, 1)
TEST_SUCCESS_FUNC(short, short, 2, 1,        div, SHRT_MIN, SHRT_MIN)
TEST_SUCCESS_FUNC(short, short, 3, SHRT_MIN, div, SHRT_MIN, 1)
TEST_SUCCESS_FUNC(short, short, 4, 1,        div, SHRT_MAX, SHRT_MAX)
TEST_SUCCESS_FUNC(short, short, 5, SHRT_MAX, div, SHRT_MAX, 1)

TEST_E_ERRNO_FUNC(short, short, 0, ERANGE, add, SHRT_MAX, 1)
TEST_E_ERRNO_FUNC(short, short, 1, ERANGE, sub, SHRT_MIN, 1)
TEST_E_ERRNO_FUNC(short, short, 2, ERANGE, mul, SHRT_MAX, 2)
TEST_E_ERRNO_FUNC(short, short, 3, ERANGE, mul, SHRT_MIN, 2)
TEST_E_ERRNO_FUNC(short, short, 4, ERANGE, mul, SHRT_MIN, -1)
TEST_E_ERRNO_FUNC(short, short, 5, EDOM,   div, 0, 0)
TEST_E_ERRNO_FUNC(short, short, 6, ERANGE, div, SHRT_MIN, -1)

/*
 * 'int'
 */

TEST_SUCCESS_FUNC(int, int,  0, 0,       add, 0, 0)
TEST_SUCCESS_FUNC(int, int,  1, 0,       add, -INT_MAX, INT_MAX)
TEST_SUCCESS_FUNC(int, int,  2, 0,       add, INT_MAX, -INT_MAX)
TEST_SUCCESS_FUNC(int, int,  3, INT_MIN, add, 0, INT_MIN)
TEST_SUCCESS_FUNC(int, int,  4, INT_MIN, add, INT_MIN, 0)
TEST_SUCCESS_FUNC(int, int,  5, INT_MIN, add, -1, INT_MIN + 1)
TEST_SUCCESS_FUNC(int, int,  6, INT_MIN, add, INT_MIN + 1, -1)
TEST_SUCCESS_FUNC(int, int,  7, INT_MAX, add, 0, INT_MAX)
TEST_SUCCESS_FUNC(int, int,  8, INT_MAX, add, INT_MAX, 0)
TEST_SUCCESS_FUNC(int, int,  9, INT_MAX, add, 1, INT_MAX - 1)
TEST_SUCCESS_FUNC(int, int, 10, INT_MAX, add, INT_MAX - 1, 1)

TEST_SUCCESS_FUNC(int, int, 0, 0,       sub, 0, 0)
TEST_SUCCESS_FUNC(int, int, 1, 0,       sub, INT_MAX, INT_MAX)
TEST_SUCCESS_FUNC(int, int, 2, 0,       sub, -INT_MAX, -INT_MAX)
TEST_SUCCESS_FUNC(int, int, 3, INT_MIN, sub, INT_MIN, 0)
TEST_SUCCESS_FUNC(int, int, 4, INT_MIN, sub, INT_MIN + 1, 1)
TEST_SUCCESS_FUNC(int, int, 5, INT_MAX, sub, INT_MAX, 0)
TEST_SUCCESS_FUNC(int, int, 6, INT_MAX, sub, INT_MAX - 1, -1)

TEST_SUCCESS_FUNC(int, int,  0, 0,           mul, 0, 0)
TEST_SUCCESS_FUNC(int, int,  1, 1,           mul, 1, 1)
TEST_SUCCESS_FUNC(int, int,  2, 0,           mul, INT_MIN, 0)
TEST_SUCCESS_FUNC(int, int,  3, 0,           mul, 0, INT_MIN)
TEST_SUCCESS_FUNC(int, int,  4, INT_MIN,     mul, INT_MIN, 1)
TEST_SUCCESS_FUNC(int, int,  5, INT_MIN,     mul, 1, INT_MIN)
TEST_SUCCESS_FUNC(int, int,  6, 0,           mul, INT_MAX, 0)
TEST_SUCCESS_FUNC(int, int,  7, 0,           mul, 0, INT_MAX)
TEST_SUCCESS_FUNC(int, int,  8, INT_MAX,     mul, INT_MAX, 1)
TEST_SUCCESS_FUNC(int, int,  9, INT_MAX,     mul, 1, INT_MAX)
TEST_SUCCESS_FUNC(int, int, 10, INT_MAX,     mul, -INT_MAX, -1)
TEST_SUCCESS_FUNC(int, int, 11, INT_MAX,     mul, -1, -INT_MAX)
TEST_SUCCESS_FUNC(int, int, 12, NEG_INT_MAX, mul, INT_MAX, -1)
TEST_SUCCESS_FUNC(int, int, 13, NEG_INT_MAX, mul, -1, INT_MAX)

TEST_SUCCESS_FUNC(int, int, 0, 0,       div, 0, 1)
TEST_SUCCESS_FUNC(int, int, 1, 1,       div, 1, 1)
TEST_SUCCESS_FUNC(int, int, 2, 1,       div, INT_MIN, INT_MIN)
TEST_SUCCESS_FUNC(int, int, 3, INT_MIN, div, INT_MIN, 1)
TEST_SUCCESS_FUNC(int, int, 4, 1,       div, INT_MAX, INT_MAX)
TEST_SUCCESS_FUNC(int, int, 5, INT_MAX, div, INT_MAX, 1)

TEST_E_ERRNO_FUNC(int, int, 0, ERANGE, add, INT_MAX, 1)
TEST_E_ERRNO_FUNC(int, int, 1, ERANGE, sub, INT_MIN, 1)
TEST_E_ERRNO_FUNC(int, int, 2, ERANGE, mul, INT_MAX, 2)
TEST_E_ERRNO_FUNC(int, int, 3, ERANGE, mul, INT_MIN, 2)
TEST_E_ERRNO_FUNC(int, int, 4, ERANGE, mul, INT_MIN, -1)
TEST_E_ERRNO_FUNC(int, int, 5, EDOM,   div, 0, 0)
TEST_E_ERRNO_FUNC(int, int, 6, ERANGE, div, INT_MIN, -1)

/*
 * 'long'
 */

TEST_SUCCESS_FUNC(long, long,  0, 0,        add, 0, 0)
TEST_SUCCESS_FUNC(long, long,  1, 0,        add, -LONG_MAX, LONG_MAX)
TEST_SUCCESS_FUNC(long, long,  2, 0,        add, LONG_MAX, -LONG_MAX)
TEST_SUCCESS_FUNC(long, long,  3, LONG_MIN, add, 0, LONG_MIN)
TEST_SUCCESS_FUNC(long, long,  4, LONG_MIN, add, LONG_MIN, 0)
TEST_SUCCESS_FUNC(long, long,  5, LONG_MIN, add, -1, LONG_MIN + 1)
TEST_SUCCESS_FUNC(long, long,  6, LONG_MIN, add, LONG_MIN + 1, -1)
TEST_SUCCESS_FUNC(long, long,  7, LONG_MAX, add, 0, LONG_MAX)
TEST_SUCCESS_FUNC(long, long,  8, LONG_MAX, add, LONG_MAX, 0)
TEST_SUCCESS_FUNC(long, long,  9, LONG_MAX, add, 1, LONG_MAX - 1)
TEST_SUCCESS_FUNC(long, long, 10, LONG_MAX, add, LONG_MAX - 1, 1)

TEST_SUCCESS_FUNC(long, long, 0, 0,        sub, 0, 0)
TEST_SUCCESS_FUNC(long, long, 1, 0,        sub, LONG_MAX, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, 2, 0,        sub, -LONG_MAX, -LONG_MAX)
TEST_SUCCESS_FUNC(long, long, 3, LONG_MIN, sub, LONG_MIN, 0)
TEST_SUCCESS_FUNC(long, long, 4, LONG_MIN, sub, LONG_MIN + 1, 1)
TEST_SUCCESS_FUNC(long, long, 5, LONG_MAX, sub, LONG_MAX, 0)
TEST_SUCCESS_FUNC(long, long, 6, LONG_MAX, sub, LONG_MAX - 1, -1)

TEST_SUCCESS_FUNC(long, long,  0, 0,            mul, 0, 0)
TEST_SUCCESS_FUNC(long, long,  1, 1,            mul, 1, 1)
TEST_SUCCESS_FUNC(long, long,  2, 0,            mul, LONG_MIN, 0)
TEST_SUCCESS_FUNC(long, long,  3, 0,            mul, 0, LONG_MIN)
TEST_SUCCESS_FUNC(long, long,  4, LONG_MIN,     mul, LONG_MIN, 1)
TEST_SUCCESS_FUNC(long, long,  5, LONG_MIN,     mul, 1, LONG_MIN)
TEST_SUCCESS_FUNC(long, long,  6, 0,            mul, LONG_MAX, 0)
TEST_SUCCESS_FUNC(long, long,  7, 0,            mul, 0, LONG_MAX)
TEST_SUCCESS_FUNC(long, long,  8, LONG_MAX,     mul, LONG_MAX, 1)
TEST_SUCCESS_FUNC(long, long,  9, LONG_MAX,     mul, 1, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, 10, LONG_MAX,     mul, -LONG_MAX, -1)
TEST_SUCCESS_FUNC(long, long, 11, LONG_MAX,     mul, -1, -LONG_MAX)
TEST_SUCCESS_FUNC(long, long, 12, NEG_LONG_MAX, mul, LONG_MAX, -1)
TEST_SUCCESS_FUNC(long, long, 13, NEG_LONG_MAX, mul, -1, LONG_MAX)

TEST_SUCCESS_FUNC(long, long, 0, 0,        div, 0, 1)
TEST_SUCCESS_FUNC(long, long, 1, 1,        div, 1, 1)
TEST_SUCCESS_FUNC(long, long, 2, 1,        div, LONG_MIN, LONG_MIN)
TEST_SUCCESS_FUNC(long, long, 3, LONG_MIN, div, LONG_MIN, 1)
TEST_SUCCESS_FUNC(long, long, 4, 1,        div, LONG_MAX, LONG_MAX)
TEST_SUCCESS_FUNC(long, long, 5, LONG_MAX, div, LONG_MAX, 1)

TEST_E_ERRNO_FUNC(long, long, 0, ERANGE, add, LONG_MAX, 1)
TEST_E_ERRNO_FUNC(long, long, 1, ERANGE, sub, LONG_MIN, 1)
TEST_E_ERRNO_FUNC(long, long, 2, ERANGE, mul, LONG_MAX, 2)
TEST_E_ERRNO_FUNC(long, long, 3, ERANGE, mul, LONG_MIN, 2)
TEST_E_ERRNO_FUNC(long, long, 4, ERANGE, mul, LONG_MIN, -1)
TEST_E_ERRNO_FUNC(long, long, 5, EDOM,   div, 0, 0)
TEST_E_ERRNO_FUNC(long, long, 6, ERANGE, div, LONG_MIN, -1)

/*
 * 'long long'
 */

TEST_SUCCESS_FUNC(llong, long long,  0, 0,         add, 0, 0)
TEST_SUCCESS_FUNC(llong, long long,  1, 0,         add, -LLONG_MAX, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long,  2, 0,         add, LLONG_MAX, -LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long,  3, LLONG_MIN, add, 0, LLONG_MIN)
TEST_SUCCESS_FUNC(llong, long long,  4, LLONG_MIN, add, LLONG_MIN, 0)
TEST_SUCCESS_FUNC(llong, long long,  5, LLONG_MIN, add, -1, LLONG_MIN + 1)
TEST_SUCCESS_FUNC(llong, long long,  6, LLONG_MIN, add, LLONG_MIN + 1, -1)
TEST_SUCCESS_FUNC(llong, long long,  7, LLONG_MAX, add, 0, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long,  8, LLONG_MAX, add, LLONG_MAX, 0)
TEST_SUCCESS_FUNC(llong, long long,  9, LLONG_MAX, add, 1, LLONG_MAX - 1)
TEST_SUCCESS_FUNC(llong, long long, 10, LLONG_MAX, add, LLONG_MAX - 1, 1)

TEST_SUCCESS_FUNC(llong, long long, 0, 0,         sub, 0, 0)
TEST_SUCCESS_FUNC(llong, long long, 1, 0,         sub, LLONG_MAX, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, 2, 0,         sub, -LLONG_MAX, -LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, 3, LLONG_MIN, sub, LLONG_MIN, 0)
TEST_SUCCESS_FUNC(llong, long long, 4, LLONG_MIN, sub, LLONG_MIN + 1, 1)
TEST_SUCCESS_FUNC(llong, long long, 5, LLONG_MAX, sub, LLONG_MAX, 0)
TEST_SUCCESS_FUNC(llong, long long, 6, LLONG_MAX, sub, LLONG_MAX - 1, -1)

TEST_SUCCESS_FUNC(llong, long long,  0, 0,             mul, 0, 0)
TEST_SUCCESS_FUNC(llong, long long,  1, 1,             mul, 1, 1)
TEST_SUCCESS_FUNC(llong, long long,  2, 0,             mul, LLONG_MIN, 0)
TEST_SUCCESS_FUNC(llong, long long,  3, 0,             mul, 0, LLONG_MIN)
TEST_SUCCESS_FUNC(llong, long long,  4, LLONG_MIN,     mul, LLONG_MIN, 1)
TEST_SUCCESS_FUNC(llong, long long,  5, LLONG_MIN,     mul, 1, LLONG_MIN)
TEST_SUCCESS_FUNC(llong, long long,  6, 0,             mul, LLONG_MAX, 0)
TEST_SUCCESS_FUNC(llong, long long,  7, 0,             mul, 0, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long,  8, LLONG_MAX,     mul, LLONG_MAX, 1)
TEST_SUCCESS_FUNC(llong, long long,  9, LLONG_MAX,     mul, 1, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, 10, LLONG_MAX,     mul, -LLONG_MAX, -1)
TEST_SUCCESS_FUNC(llong, long long, 11, LLONG_MAX,     mul, -1, -LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, 12, NEG_LLONG_MAX, mul, LLONG_MAX, -1)
TEST_SUCCESS_FUNC(llong, long long, 13, NEG_LLONG_MAX, mul, -1, LLONG_MAX)

TEST_SUCCESS_FUNC(llong, long long, 0, 0,         div, 0, 1)
TEST_SUCCESS_FUNC(llong, long long, 1, 1,         div, 1, 1)
TEST_SUCCESS_FUNC(llong, long long, 2, 1,         div, LLONG_MIN, LLONG_MIN)
TEST_SUCCESS_FUNC(llong, long long, 3, LLONG_MIN, div, LLONG_MIN, 1)
TEST_SUCCESS_FUNC(llong, long long, 4, 1,         div, LLONG_MAX, LLONG_MAX)
TEST_SUCCESS_FUNC(llong, long long, 5, LLONG_MAX, div, LLONG_MAX, 1)

TEST_E_ERRNO_FUNC(llong, long long, 0, ERANGE, add, LLONG_MAX, 1)
TEST_E_ERRNO_FUNC(llong, long long, 1, ERANGE, sub, LLONG_MIN, 1)
TEST_E_ERRNO_FUNC(llong, long long, 2, ERANGE, mul, LLONG_MAX, 2)
TEST_E_ERRNO_FUNC(llong, long long, 3, ERANGE, mul, LLONG_MIN, 2)
TEST_E_ERRNO_FUNC(llong, long long, 4, ERANGE, mul, LLONG_MIN, -1)
TEST_E_ERRNO_FUNC(llong, long long, 5, EDOM,   div, 0, 0)
TEST_E_ERRNO_FUNC(llong, long long, 6, ERANGE, div, LLONG_MIN, -1)

/*
 * 'unsigned char'
 */

TEST_SUCCESS_FUNC(uchar, unsigned char, 0, 0,         add, 0, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, 1, UCHAR_MAX, add, 0, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, 2, UCHAR_MAX, add, UCHAR_MAX, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, 3, UCHAR_MAX, add, 1, UCHAR_MAX - 1)
TEST_SUCCESS_FUNC(uchar, unsigned char, 4, UCHAR_MAX, add, UCHAR_MAX - 1, 1)

TEST_SUCCESS_FUNC(uchar, unsigned char, 0, 0,         sub, 0, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char, 1, 1,         sub, UCHAR_MAX, UCHAR_MAX - 1)
TEST_SUCCESS_FUNC(uchar, unsigned char, 2, UCHAR_MAX, sub, UCHAR_MAX, 0)

TEST_SUCCESS_FUNC(uchar, unsigned char,  0, 0,         mul, 0, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char,  1, 1,         mul, 1, 1)
TEST_SUCCESS_FUNC(uchar, unsigned char,  2, 0,         mul, UCHAR_MAX, 0)
TEST_SUCCESS_FUNC(uchar, unsigned char,  3, 0,         mul, 0, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char,  4, UCHAR_MAX, mul, UCHAR_MAX, 1)
TEST_SUCCESS_FUNC(uchar, unsigned char,  5, UCHAR_MAX, mul, 1, UCHAR_MAX)

TEST_SUCCESS_FUNC(uchar, unsigned char, 0, 0,         div, 0, 1)
TEST_SUCCESS_FUNC(uchar, unsigned char, 1, 1,         div, 1, 1)
TEST_SUCCESS_FUNC(uchar, unsigned char, 2, 1,         div, UCHAR_MAX, UCHAR_MAX)
TEST_SUCCESS_FUNC(uchar, unsigned char, 3, UCHAR_MAX, div, UCHAR_MAX, 1)

TEST_E_ERRNO_FUNC(uchar, unsigned char, 0, ERANGE, add, UCHAR_MAX, 1)
TEST_E_ERRNO_FUNC(uchar, unsigned char, 1, ERANGE, sub, 0, 1)
TEST_E_ERRNO_FUNC(uchar, unsigned char, 2, ERANGE, mul, UCHAR_MAX, 2)
TEST_E_ERRNO_FUNC(uchar, unsigned char, 3, EDOM,   div, 0, 0)

/*
 * 'unsigned short'
 */

TEST_SUCCESS_FUNC(ushort, unsigned short, 0, 0,         add, 0, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, 1, USHRT_MAX, add, 0, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, 2, USHRT_MAX, add, USHRT_MAX, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, 3, USHRT_MAX, add, 1, USHRT_MAX - 1)
TEST_SUCCESS_FUNC(ushort, unsigned short, 4, USHRT_MAX, add, USHRT_MAX - 1, 1)

TEST_SUCCESS_FUNC(ushort, unsigned short, 0, 0,         sub, 0, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short, 1, 1,         sub, USHRT_MAX, USHRT_MAX - 1)
TEST_SUCCESS_FUNC(ushort, unsigned short, 2, USHRT_MAX, sub, USHRT_MAX, 0)

TEST_SUCCESS_FUNC(ushort, unsigned short,  0, 0,         mul, 0, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short,  1, 1,         mul, 1, 1)
TEST_SUCCESS_FUNC(ushort, unsigned short,  2, 0,         mul, USHRT_MAX, 0)
TEST_SUCCESS_FUNC(ushort, unsigned short,  3, 0,         mul, 0, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short,  4, USHRT_MAX, mul, USHRT_MAX, 1)
TEST_SUCCESS_FUNC(ushort, unsigned short,  5, USHRT_MAX, mul, 1, USHRT_MAX)

TEST_SUCCESS_FUNC(ushort, unsigned short, 0, 0,         div, 0, 1)
TEST_SUCCESS_FUNC(ushort, unsigned short, 1, 1,         div, 1, 1)
TEST_SUCCESS_FUNC(ushort, unsigned short, 2, 1,         div, USHRT_MAX, USHRT_MAX)
TEST_SUCCESS_FUNC(ushort, unsigned short, 3, USHRT_MAX, div, USHRT_MAX, 1)

TEST_E_ERRNO_FUNC(ushort, unsigned short, 0, ERANGE, add, USHRT_MAX, 1)
TEST_E_ERRNO_FUNC(ushort, unsigned short, 1, ERANGE, sub, 0, 1)
TEST_E_ERRNO_FUNC(ushort, unsigned short, 2, ERANGE, mul, USHRT_MAX, 2)
TEST_E_ERRNO_FUNC(ushort, unsigned short, 3, EDOM,   div, 0, 0)

/*
 * 'unsigned int'
 */

TEST_SUCCESS_FUNC(uint, unsigned int, 0, 0,        add, 0, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, 1, UINT_MAX, add, 0, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, 2, UINT_MAX, add, UINT_MAX, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, 3, UINT_MAX, add, 1, UINT_MAX - 1)
TEST_SUCCESS_FUNC(uint, unsigned int, 4, UINT_MAX, add, UINT_MAX - 1, 1)

TEST_SUCCESS_FUNC(uint, unsigned int, 0, 0,        sub, 0, 0)
TEST_SUCCESS_FUNC(uint, unsigned int, 1, 1,        sub, UINT_MAX, UINT_MAX - 1)
TEST_SUCCESS_FUNC(uint, unsigned int, 2, UINT_MAX, sub, UINT_MAX, 0)

TEST_SUCCESS_FUNC(uint, unsigned int,  0, 0,        mul, 0, 0)
TEST_SUCCESS_FUNC(uint, unsigned int,  1, 1,        mul, 1, 1)
TEST_SUCCESS_FUNC(uint, unsigned int,  2, 0,        mul, UINT_MAX, 0)
TEST_SUCCESS_FUNC(uint, unsigned int,  3, 0,        mul, 0, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int,  4, UINT_MAX, mul, UINT_MAX, 1)
TEST_SUCCESS_FUNC(uint, unsigned int,  5, UINT_MAX, mul, 1, UINT_MAX)

TEST_SUCCESS_FUNC(uint, unsigned int, 0, 0,        div, 0, 1)
TEST_SUCCESS_FUNC(uint, unsigned int, 1, 1,        div, 1, 1)
TEST_SUCCESS_FUNC(uint, unsigned int, 2, 1,        div, UINT_MAX, UINT_MAX)
TEST_SUCCESS_FUNC(uint, unsigned int, 3, UINT_MAX, div, UINT_MAX, 1)

TEST_E_ERRNO_FUNC(uint, unsigned int, 0, ERANGE, add, UINT_MAX, 1)
TEST_E_ERRNO_FUNC(uint, unsigned int, 1, ERANGE, sub, 0, 1)
TEST_E_ERRNO_FUNC(uint, unsigned int, 2, ERANGE, mul, UINT_MAX, 2)
TEST_E_ERRNO_FUNC(uint, unsigned int, 3, EDOM,   div, 0, 0)

/*
 * 'unsigned long'
 */

TEST_SUCCESS_FUNC(ulong, unsigned long, 0, 0,         add, 0, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, 1, ULONG_MAX, add, 0, ULONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, 2, ULONG_MAX, add, ULONG_MAX, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, 3, ULONG_MAX, add, 1, ULONG_MAX - 1)
TEST_SUCCESS_FUNC(ulong, unsigned long, 4, ULONG_MAX, add, ULONG_MAX - 1, 1)

TEST_SUCCESS_FUNC(ulong, unsigned long, 0, 0,         sub, 0, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long, 1, 1,         sub, ULONG_MAX, ULONG_MAX - 1)
TEST_SUCCESS_FUNC(ulong, unsigned long, 2, ULONG_MAX, sub, ULONG_MAX, 0)

TEST_SUCCESS_FUNC(ulong, unsigned long,  0, 0,         mul, 0, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long,  1, 1,         mul, 1, 1)
TEST_SUCCESS_FUNC(ulong, unsigned long,  2, 0,         mul, ULONG_MAX, 0)
TEST_SUCCESS_FUNC(ulong, unsigned long,  3, 0,         mul, 0, ULONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long,  4, ULONG_MAX, mul, ULONG_MAX, 1)
TEST_SUCCESS_FUNC(ulong, unsigned long,  5, ULONG_MAX, mul, 1, ULONG_MAX)

TEST_SUCCESS_FUNC(ulong, unsigned long, 0, 0,         div, 0, 1)
TEST_SUCCESS_FUNC(ulong, unsigned long, 1, 1,         div, 1, 1)
TEST_SUCCESS_FUNC(ulong, unsigned long, 2, 1,         div, ULONG_MAX, ULONG_MAX)
TEST_SUCCESS_FUNC(ulong, unsigned long, 3, ULONG_MAX, div, ULONG_MAX, 1)

TEST_E_ERRNO_FUNC(ulong, unsigned long, 0, ERANGE, add, ULONG_MAX, 1)
TEST_E_ERRNO_FUNC(ulong, unsigned long, 1, ERANGE, sub, 0, 1)
TEST_E_ERRNO_FUNC(ulong, unsigned long, 2, ERANGE, mul, ULONG_MAX, 2)
TEST_E_ERRNO_FUNC(ulong, unsigned long, 3, EDOM,   div, 0, 0)

/*
 * 'unsigned long long'
 */

TEST_SUCCESS_FUNC(ullong, unsigned long long, 0, 0,          add, 0, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 1, ULLONG_MAX, add, 0, ULLONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 2, ULLONG_MAX, add, ULLONG_MAX, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 3, ULLONG_MAX, add, 1, ULLONG_MAX - 1)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 4, ULLONG_MAX, add, ULLONG_MAX - 1, 1)

TEST_SUCCESS_FUNC(ullong, unsigned long long, 0, 0,          sub, 0, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 1, 1,          sub, ULLONG_MAX, ULLONG_MAX - 1)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 2, ULLONG_MAX, sub, ULLONG_MAX, 0)

TEST_SUCCESS_FUNC(ullong, unsigned long long,  0, 0,          mul, 0, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long,  1, 1,          mul, 1, 1)
TEST_SUCCESS_FUNC(ullong, unsigned long long,  2, 0,          mul, ULLONG_MAX, 0)
TEST_SUCCESS_FUNC(ullong, unsigned long long,  3, 0,          mul, 0, ULLONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long,  4, ULLONG_MAX, mul, ULLONG_MAX, 1)
TEST_SUCCESS_FUNC(ullong, unsigned long long,  5, ULLONG_MAX, mul, 1, ULLONG_MAX)

TEST_SUCCESS_FUNC(ullong, unsigned long long, 0, 0,          div, 0, 1)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 1, 1,          div, 1, 1)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 2, 1,          div, ULLONG_MAX, ULLONG_MAX)
TEST_SUCCESS_FUNC(ullong, unsigned long long, 3, ULLONG_MAX, div, ULLONG_MAX, 1)

TEST_E_ERRNO_FUNC(ullong, unsigned long long, 0, ERANGE, add, ULLONG_MAX, 1)
TEST_E_ERRNO_FUNC(ullong, unsigned long long, 1, ERANGE, sub, 0, 1)
TEST_E_ERRNO_FUNC(ullong, unsigned long long, 2, ERANGE, mul, ULLONG_MAX, 2)
TEST_E_ERRNO_FUNC(ullong, unsigned long long, 3, EDOM,   div, 0, 0)

/*
 * 'float'
 */

TEST_SUCCESS_FUNC(float, float,  0, 0,           add, 0.f, 0.f)
TEST_SUCCESS_FUNC(float, float,  1, 0,           add, NEG_FLT_EXT, POS_FLT_EXT)
TEST_SUCCESS_FUNC(float, float,  2, 0,           add, POS_FLT_EXT, NEG_FLT_EXT)
TEST_SUCCESS_FUNC(float, float,  3, NEG_FLT_EXT, add, 0.f, NEG_FLT_EXT)
TEST_SUCCESS_FUNC(float, float,  4, NEG_FLT_EXT, add, NEG_FLT_EXT, 0.f)
TEST_SUCCESS_FUNC(float, float,  5, NEG_TWO,     add, -1.f, -1.f)
TEST_SUCCESS_FUNC(float, float,  6, POS_FLT_EXT, add, 0.f, POS_FLT_EXT)
TEST_SUCCESS_FUNC(float, float,  7, POS_FLT_EXT, add, POS_FLT_EXT, 0.f)
TEST_SUCCESS_FUNC(float, float,  8, 2,           add, 1.f, 1.f)

TEST_SUCCESS_FUNC(float, float, 0, 0,           sub, 0.f, 0.f)
TEST_SUCCESS_FUNC(float, float, 1, 0,           sub, POS_FLT_EXT, POS_FLT_EXT)
TEST_SUCCESS_FUNC(float, float, 2, 0,           sub, NEG_FLT_EXT, NEG_FLT_EXT)
TEST_SUCCESS_FUNC(float, float, 3, NEG_FLT_EXT, sub, NEG_FLT_EXT, 0.f)
TEST_SUCCESS_FUNC(float, float, 4, NEG_FLT_EXT, sub, NEG_FLT_EXT + 1.f, 1.f)
TEST_SUCCESS_FUNC(float, float, 5, POS_FLT_EXT, sub, POS_FLT_EXT, 0.f)
TEST_SUCCESS_FUNC(float, float, 6, POS_FLT_EXT, sub, POS_FLT_EXT - 1.f, -1.f)

TEST_SUCCESS_FUNC(float, float,  0, 0,           mul, 0.f, 0.f)
TEST_SUCCESS_FUNC(float, float,  1, 1,           mul, 1.f, 1.f)
TEST_SUCCESS_FUNC(float, float,  2, 0,           mul, NEG_FLT_EXT, 0.f)
TEST_SUCCESS_FUNC(float, float,  3, 0,           mul, 0.f, NEG_FLT_EXT)
TEST_SUCCESS_FUNC(float, float,  4, NEG_FLT_EXT, mul, NEG_FLT_EXT, 1.f)
TEST_SUCCESS_FUNC(float, float,  5, NEG_FLT_EXT, mul, 1.f, NEG_FLT_EXT)
TEST_SUCCESS_FUNC(float, float,  6, 0,           mul, POS_FLT_EXT, 0.f)
TEST_SUCCESS_FUNC(float, float,  7, 0,           mul, 0.f, POS_FLT_EXT)
TEST_SUCCESS_FUNC(float, float,  8, POS_FLT_EXT, mul, POS_FLT_EXT, 1)
TEST_SUCCESS_FUNC(float, float,  9, POS_FLT_EXT, mul, 1.f, POS_FLT_EXT)
TEST_SUCCESS_FUNC(float, float, 10, POS_FLT_EXT, mul, NEG_FLT_EXT, -1.f)
TEST_SUCCESS_FUNC(float, float, 11, POS_FLT_EXT, mul, -1.f, NEG_FLT_EXT)
TEST_SUCCESS_FUNC(float, float, 12, NEG_FLT_EXT, mul, POS_FLT_EXT, -1.f)
TEST_SUCCESS_FUNC(float, float, 13, NEG_FLT_EXT, mul, -1.f, POS_FLT_EXT)

TEST_SUCCESS_FUNC(float, float, 0, 0,           div, 0.f, 1.f)
TEST_SUCCESS_FUNC(float, float, 1, 1,           div, 1.f, 1.f)
TEST_SUCCESS_FUNC(float, float, 2, 1,           div, NEG_FLT_EXT, NEG_FLT_EXT)
TEST_SUCCESS_FUNC(float, float, 3, NEG_FLT_EXT, div, NEG_FLT_EXT, 1.f)
TEST_SUCCESS_FUNC(float, float, 4, 1,           div, POS_FLT_EXT, POS_FLT_EXT)
TEST_SUCCESS_FUNC(float, float, 5, POS_FLT_EXT, div, POS_FLT_EXT, 1.f)

TEST_E_ERRNO_FUNC(float, float,  0, ERANGE, add, POS_FLT_EXT,  POS_FLT_EXT / 2.f)
TEST_E_ERRNO_FUNC(float, float,  1, ERANGE, sub, NEG_FLT_EXT,  POS_FLT_EXT / 2.f)
TEST_E_ERRNO_FUNC(float, float,  2, ERANGE, mul, POS_FLT_EXT,  2.f)
TEST_E_ERRNO_FUNC(float, float,  3, ERANGE, mul, POS_FLT_EXT, -2.f)
TEST_E_ERRNO_FUNC(float, float,  4, ERANGE, mul, NEG_FLT_EXT,  2.f)
TEST_E_ERRNO_FUNC(float, float,  5, ERANGE, mul, NEG_FLT_EXT, -2.f)
TEST_E_ERRNO_FUNC(float, float,  6, EDOM,   div, 0.f, 0.f)
TEST_E_ERRNO_FUNC(float, float,  7, ERANGE, div, POS_FLT_EXT,  0.5f)
TEST_E_ERRNO_FUNC(float, float,  8, ERANGE, div, POS_FLT_EXT, -0.5f)
TEST_E_ERRNO_FUNC(float, float,  9, ERANGE, div, NEG_FLT_EXT,  0.5f)
TEST_E_ERRNO_FUNC(float, float, 10, ERANGE, div, NEG_FLT_EXT, -0.5f)

/*
 * 'double'
 */

TEST_SUCCESS_FUNC(double, double,  0, 0,           add, 0., 0.)
TEST_SUCCESS_FUNC(double, double,  1, 0,           add, NEG_DBL_EXT, POS_DBL_EXT)
TEST_SUCCESS_FUNC(double, double,  2, 0,           add, POS_DBL_EXT, NEG_DBL_EXT)
TEST_SUCCESS_FUNC(double, double,  3, NEG_DBL_EXT, add, 0., NEG_DBL_EXT)
TEST_SUCCESS_FUNC(double, double,  4, NEG_DBL_EXT, add, NEG_DBL_EXT, 0.)
TEST_SUCCESS_FUNC(double, double,  5, NEG_TWO,     add, -1., -1.)
TEST_SUCCESS_FUNC(double, double,  6, POS_DBL_EXT, add, 0., POS_DBL_EXT)
TEST_SUCCESS_FUNC(double, double,  7, POS_DBL_EXT, add, POS_DBL_EXT, 0.)
TEST_SUCCESS_FUNC(double, double,  8, 2,           add, 1., 1.)

TEST_SUCCESS_FUNC(double, double, 0, 0,           sub, 0., 0.)
TEST_SUCCESS_FUNC(double, double, 1, 0,           sub, POS_DBL_EXT, POS_DBL_EXT)
TEST_SUCCESS_FUNC(double, double, 2, 0,           sub, NEG_DBL_EXT, NEG_DBL_EXT)
TEST_SUCCESS_FUNC(double, double, 3, NEG_DBL_EXT, sub, NEG_DBL_EXT, 0.)
TEST_SUCCESS_FUNC(double, double, 4, NEG_DBL_EXT, sub, NEG_DBL_EXT + 1., 1.)
TEST_SUCCESS_FUNC(double, double, 5, POS_DBL_EXT, sub, POS_DBL_EXT, 0.)
TEST_SUCCESS_FUNC(double, double, 6, POS_DBL_EXT, sub, POS_DBL_EXT - 1., -1.)

TEST_SUCCESS_FUNC(double, double,  0, 0,           mul, 0., 0.)
TEST_SUCCESS_FUNC(double, double,  1, 1,           mul, 1., 1.)
TEST_SUCCESS_FUNC(double, double,  2, 0,           mul, NEG_DBL_EXT, 0.)
TEST_SUCCESS_FUNC(double, double,  3, 0,           mul, 0., NEG_DBL_EXT)
TEST_SUCCESS_FUNC(double, double,  4, NEG_DBL_EXT, mul, NEG_DBL_EXT, 1.)
TEST_SUCCESS_FUNC(double, double,  5, NEG_DBL_EXT, mul, 1., NEG_DBL_EXT)
TEST_SUCCESS_FUNC(double, double,  6, 0,           mul, POS_DBL_EXT, 0.)
TEST_SUCCESS_FUNC(double, double,  7, 0,           mul, 0., POS_DBL_EXT)
TEST_SUCCESS_FUNC(double, double,  8, POS_DBL_EXT, mul, POS_DBL_EXT, 1.)
TEST_SUCCESS_FUNC(double, double,  9, POS_DBL_EXT, mul, 1., POS_DBL_EXT)
TEST_SUCCESS_FUNC(double, double, 10, POS_DBL_EXT, mul, NEG_DBL_EXT, -1.)
TEST_SUCCESS_FUNC(double, double, 11, POS_DBL_EXT, mul, -1., NEG_DBL_EXT)
TEST_SUCCESS_FUNC(double, double, 12, NEG_DBL_EXT, mul, POS_DBL_EXT, -1.)
TEST_SUCCESS_FUNC(double, double, 13, NEG_DBL_EXT, mul, -1., POS_DBL_EXT)

TEST_SUCCESS_FUNC(double, double, 0, 0,           div, 0., 1.)
TEST_SUCCESS_FUNC(double, double, 1, 1,           div, 1., 1.)
TEST_SUCCESS_FUNC(double, double, 2, 1,           div, NEG_DBL_EXT, NEG_DBL_EXT)
TEST_SUCCESS_FUNC(double, double, 3, NEG_DBL_EXT, div, NEG_DBL_EXT, 1.)
TEST_SUCCESS_FUNC(double, double, 4, 1,           div, POS_DBL_EXT, POS_DBL_EXT)
TEST_SUCCESS_FUNC(double, double, 5, POS_DBL_EXT, div, POS_DBL_EXT, 1.)

TEST_E_ERRNO_FUNC(double, double,  0, ERANGE, add, POS_DBL_EXT,  POS_DBL_EXT / 2.)
TEST_E_ERRNO_FUNC(double, double,  1, ERANGE, sub, NEG_DBL_EXT,  POS_DBL_EXT / 2.)
TEST_E_ERRNO_FUNC(double, double,  2, ERANGE, mul, POS_DBL_EXT,  2.)
TEST_E_ERRNO_FUNC(double, double,  3, ERANGE, mul, POS_DBL_EXT, -2.)
TEST_E_ERRNO_FUNC(double, double,  4, ERANGE, mul, NEG_DBL_EXT,  2.)
TEST_E_ERRNO_FUNC(double, double,  5, ERANGE, mul, NEG_DBL_EXT, -2.)
TEST_E_ERRNO_FUNC(double, double,  6, EDOM,   div, 0., 0.)
TEST_E_ERRNO_FUNC(double, double,  7, ERANGE, div, POS_DBL_EXT,  0.5)
TEST_E_ERRNO_FUNC(double, double,  8, ERANGE, div, POS_DBL_EXT, -0.5)
TEST_E_ERRNO_FUNC(double, double,  9, ERANGE, div, NEG_DBL_EXT,  0.5)
TEST_E_ERRNO_FUNC(double, double, 10, ERANGE, div, NEG_DBL_EXT, -0.5)

/*
 * 'long double'
 *
 * Valgrind does not support 'long double'. Internal computations
 * are performed with double precision. This is a known limitation.
 *
 * Cygwin does not reliably support 'long double'. We skip the tests
 * that are known to be broken.
 */

TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  0, 0,            add, 0.l, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  1, 0,            add, NEG_LDBL_EXT, POS_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  2, 0,            add, POS_LDBL_EXT, NEG_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  3, NEG_LDBL_EXT, add, 0.l, NEG_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  4, NEG_LDBL_EXT, add, NEG_LDBL_EXT, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  5, NEG_TWO,      add, -1.l, -1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double,  6, POS_LDBL_EXT, add, 0.l, POS_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  7, POS_LDBL_EXT, add, POS_LDBL_EXT, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  8, 2,            add, 1.l, 1.l)

TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 0, 0,            sub, 0.l, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 1, 0,            sub, POS_LDBL_EXT, POS_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 2, 0,            sub, NEG_LDBL_EXT, NEG_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 3, NEG_LDBL_EXT, sub, NEG_LDBL_EXT, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 4, NEG_LDBL_EXT, sub, NEG_LDBL_EXT + 1.l, 1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 5, POS_LDBL_EXT, sub, POS_LDBL_EXT, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 6, POS_LDBL_EXT, sub, POS_LDBL_EXT - 1.l, -1.l)

TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  0, 0,            mul, 0.l, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  1, 1,            mul, 1.l, 1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  2, 0,            mul, NEG_LDBL_EXT, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  3, 0,            mul, 0.l, NEG_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  4, NEG_LDBL_EXT, mul, NEG_LDBL_EXT, 1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double,  5, NEG_LDBL_EXT, mul, 1.l, NEG_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  6, 0,            mul, POS_LDBL_EXT, 0.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  7, 0,            mul, 0.l, POS_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double,  8, POS_LDBL_EXT, mul, POS_LDBL_EXT, 1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double,  9, POS_LDBL_EXT, mul, 1.l, POS_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 10, POS_LDBL_EXT, mul, NEG_LDBL_EXT, -1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 11, POS_LDBL_EXT, mul, -1.l, NEG_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 12, NEG_LDBL_EXT, mul, POS_LDBL_EXT, -1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 13, NEG_LDBL_EXT, mul, -1.l, POS_LDBL_EXT)

TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 0, 0,            div, 0.l, 1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 1, 1,            div, 1.l, 1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 2, 1,            div, NEG_LDBL_EXT, NEG_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 3, NEG_LDBL_EXT, div, NEG_LDBL_EXT, 1.l)
TEST_SUCCESS_FUNC_IF(!is_valgrind(),                 ldouble, long double, 4, 1,            div, POS_LDBL_EXT, POS_LDBL_EXT)
TEST_SUCCESS_FUNC_IF(!is_valgrind() && !is_cygwin(), ldouble, long double, 5, POS_LDBL_EXT, div, POS_LDBL_EXT, 1.l)

TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  0, ERANGE, add, POS_LDBL_EXT,  POS_LDBL_EXT / 2.f)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  1, ERANGE, sub, NEG_LDBL_EXT,  POS_LDBL_EXT / 2.f)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  2, ERANGE, mul, POS_LDBL_EXT,  2.l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  3, ERANGE, mul, POS_LDBL_EXT, -2.l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  4, ERANGE, mul, NEG_LDBL_EXT,  2.l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  5, ERANGE, mul, NEG_LDBL_EXT, -2.l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  6, EDOM,   div, 0.l, 0.l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  7, ERANGE, div, POS_LDBL_EXT,  0.5l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  8, ERANGE, div, POS_LDBL_EXT, -0.5l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double,  9, ERANGE, div, NEG_LDBL_EXT,  0.5l)
TEST_E_ERRNO_FUNC_IF(!is_valgrind(), ldouble, long double, 10, ERANGE, div, NEG_LDBL_EXT, -0.5l)

/*
 * Tests
 */

static const struct test_func test[] = {
    /* '_Bool' */
    TEST_SUCCESS(0, _Bool, false, add),
    TEST_SUCCESS(1, _Bool, true,  add),
    TEST_SUCCESS(2, _Bool, true,  add),
    TEST_SUCCESS(0, _Bool, false, sub),
    TEST_SUCCESS(1, _Bool, false, sub),
    TEST_SUCCESS(2, _Bool, true,  sub),
    TEST_SUCCESS(0, _Bool, false, mul),
    TEST_SUCCESS(1, _Bool, false, mul),
    TEST_SUCCESS(2, _Bool, false, mul),
    TEST_SUCCESS(3, _Bool, true,  mul),
    TEST_SUCCESS(0, _Bool, false, div),
    TEST_SUCCESS(1, _Bool, true,  div),
    TEST_E_ERRNO(0, _Bool, ERANGE, add),
    TEST_E_ERRNO(1, _Bool, ERANGE, sub),
    /* 'char' */
    TEST_SUCCESS(0, char, 0,        add),
    TEST_SUCCESS(1, char, CHAR_MIN, add),
    TEST_SUCCESS(2, char, CHAR_MIN, add),
    TEST_SUCCESS(3, char, CHAR_MAX, add),
    TEST_SUCCESS(4, char, CHAR_MAX, add),
    TEST_SUCCESS(5, char, CHAR_MAX, add),
    TEST_SUCCESS(6, char, CHAR_MAX, add),
    TEST_SUCCESS(0, char, 0,        sub),
    TEST_SUCCESS(1, char, CHAR_MIN, sub),
    TEST_SUCCESS(2, char, CHAR_MIN, sub),
    TEST_SUCCESS(3, char, CHAR_MAX, sub),
    TEST_SUCCESS(0, char, 0,        mul),
    TEST_SUCCESS(1, char, 1,        mul),
    TEST_SUCCESS(2, char, 0,        mul),
    TEST_SUCCESS(3, char, 0,        mul),
    TEST_SUCCESS(4, char, CHAR_MIN, mul),
    TEST_SUCCESS(5, char, CHAR_MIN, mul),
    TEST_SUCCESS(6, char, 0,        mul),
    TEST_SUCCESS(7, char, 0,        mul),
    TEST_SUCCESS(8, char, CHAR_MAX, mul),
    TEST_SUCCESS(9, char, CHAR_MAX, mul),
    TEST_SUCCESS(0, char, 0,        div),
    TEST_SUCCESS(1, char, 1,        div),
    TEST_SUCCESS(2, char, 1,        div),
    TEST_SUCCESS(3, char, CHAR_MAX, div),
    TEST_E_ERRNO(0, char, ERANGE, add),
    TEST_E_ERRNO(1, char, ERANGE, sub),
    TEST_E_ERRNO(2, char, ERANGE, mul),
    TEST_E_ERRNO(3, char, EDOM,   div),
    /* 'signed char' */
    TEST_SUCCESS( 0, schar, 0, add),
    TEST_SUCCESS( 1, schar, 0, add),
    TEST_SUCCESS( 2, schar, 0, add),
    TEST_SUCCESS( 3, schar, SCHAR_MIN, add),
    TEST_SUCCESS( 4, schar, SCHAR_MIN, add),
    TEST_SUCCESS( 5, schar, SCHAR_MIN, add),
    TEST_SUCCESS( 6, schar, SCHAR_MIN, add),
    TEST_SUCCESS( 7, schar, SCHAR_MAX, add),
    TEST_SUCCESS( 8, schar, SCHAR_MAX, add),
    TEST_SUCCESS( 9, schar, SCHAR_MAX, add),
    TEST_SUCCESS(10, schar, SCHAR_MAX, add),
    TEST_SUCCESS( 0, schar, 0, sub),
    TEST_SUCCESS( 1, schar, 0, sub),
    TEST_SUCCESS( 2, schar, 0, sub),
    TEST_SUCCESS( 3, schar, SCHAR_MIN, sub),
    TEST_SUCCESS( 4, schar, SCHAR_MIN, sub),
    TEST_SUCCESS( 5, schar, SCHAR_MAX, sub),
    TEST_SUCCESS( 6, schar, SCHAR_MAX, sub),
    TEST_SUCCESS( 0, schar, 0, mul),
    TEST_SUCCESS( 1, schar, 1, mul),
    TEST_SUCCESS( 2, schar, 0, mul),
    TEST_SUCCESS( 3, schar, 0, mul),
    TEST_SUCCESS( 4, schar, SCHAR_MIN, mul),
    TEST_SUCCESS( 5, schar, SCHAR_MIN, mul),
    TEST_SUCCESS( 6, schar, 0, mul),
    TEST_SUCCESS( 7, schar, 0, mul),
    TEST_SUCCESS( 8, schar, SCHAR_MAX, mul),
    TEST_SUCCESS( 9, schar, SCHAR_MAX, mul),
    TEST_SUCCESS(10, schar, SCHAR_MAX, mul),
    TEST_SUCCESS(11, schar, SCHAR_MAX, mul),
    TEST_SUCCESS(12, schar, NEG_SCHAR_MAX, mul),
    TEST_SUCCESS(13, schar, NEG_SCHAR_MAX, mul),
    TEST_SUCCESS( 0, schar, 0,         div),
    TEST_SUCCESS( 1, schar, 1,         div),
    TEST_SUCCESS( 2, schar, 1,         div),
    TEST_SUCCESS( 3, schar, SCHAR_MIN, div),
    TEST_SUCCESS( 4, schar, 1,         div),
    TEST_SUCCESS( 5, schar, SCHAR_MAX, div),
    TEST_E_ERRNO( 0, schar, ERANGE, add),
    TEST_E_ERRNO( 1, schar, ERANGE, sub),
    TEST_E_ERRNO( 2, schar, ERANGE, mul),
    TEST_E_ERRNO( 3, schar, ERANGE, mul),
    TEST_E_ERRNO( 4, schar, ERANGE, mul),
    TEST_E_ERRNO( 5, schar, EDOM,   div),
    TEST_E_ERRNO( 6, schar, ERANGE, div),
    /* 'short' */
    TEST_SUCCESS( 0, short, 0,        add),
    TEST_SUCCESS( 1, short, 0,        add),
    TEST_SUCCESS( 2, short, 0,        add),
    TEST_SUCCESS( 3, short, SHRT_MIN, add),
    TEST_SUCCESS( 4, short, SHRT_MIN, add),
    TEST_SUCCESS( 5, short, SHRT_MIN, add),
    TEST_SUCCESS( 6, short, SHRT_MIN, add),
    TEST_SUCCESS( 7, short, SHRT_MAX, add),
    TEST_SUCCESS( 8, short, SHRT_MAX, add),
    TEST_SUCCESS( 9, short, SHRT_MAX, add),
    TEST_SUCCESS(10, short, SHRT_MAX, add),
    TEST_SUCCESS( 0, short, 0,        sub),
    TEST_SUCCESS( 1, short, 0,        sub),
    TEST_SUCCESS( 2, short, 0,        sub),
    TEST_SUCCESS( 3, short, SHRT_MIN, sub),
    TEST_SUCCESS( 4, short, SHRT_MIN, sub),
    TEST_SUCCESS( 5, short, SHRT_MAX, sub),
    TEST_SUCCESS( 6, short, SHRT_MAX, sub),
    TEST_SUCCESS( 0, short, 0, mul),
    TEST_SUCCESS( 1, short, 1, mul),
    TEST_SUCCESS( 2, short, 0, mul),
    TEST_SUCCESS( 3, short, 0, mul),
    TEST_SUCCESS( 4, short, SHRT_MIN, mul),
    TEST_SUCCESS( 5, short, SHRT_MIN, mul),
    TEST_SUCCESS( 6, short, 0, mul),
    TEST_SUCCESS( 7, short, 0, mul),
    TEST_SUCCESS( 8, short, SHRT_MAX, mul),
    TEST_SUCCESS( 9, short, SHRT_MAX, mul),
    TEST_SUCCESS(10, short, SHRT_MAX, mul),
    TEST_SUCCESS(11, short, SHRT_MAX, mul),
    TEST_SUCCESS(12, short, NEG_SHRT_MAX, mul),
    TEST_SUCCESS(13, short, NEG_SHRT_MAX, mul),
    TEST_SUCCESS( 0, short, 0,        div),
    TEST_SUCCESS( 1, short, 1,        div),
    TEST_SUCCESS( 2, short, 1,        div),
    TEST_SUCCESS( 3, short, SHRT_MIN, div),
    TEST_SUCCESS( 4, short, 1,        div),
    TEST_SUCCESS( 5, short, SHRT_MAX, div),
    TEST_E_ERRNO( 0, short, ERANGE, add),
    TEST_E_ERRNO( 1, short, ERANGE, sub),
    TEST_E_ERRNO( 2, short, ERANGE, mul),
    TEST_E_ERRNO( 3, short, ERANGE, mul),
    TEST_E_ERRNO( 4, short, ERANGE, mul),
    TEST_E_ERRNO( 5, short, EDOM,   div),
    TEST_E_ERRNO( 6, short, ERANGE, div),
    /* 'int' */
    TEST_SUCCESS( 0, int, 0,       add),
    TEST_SUCCESS( 1, int, 0,       add),
    TEST_SUCCESS( 2, int, 0,       add),
    TEST_SUCCESS( 3, int, INT_MIN, add),
    TEST_SUCCESS( 4, int, INT_MIN, add),
    TEST_SUCCESS( 5, int, INT_MIN, add),
    TEST_SUCCESS( 6, int, INT_MIN, add),
    TEST_SUCCESS( 7, int, INT_MAX, add),
    TEST_SUCCESS( 8, int, INT_MAX, add),
    TEST_SUCCESS( 9, int, INT_MAX, add),
    TEST_SUCCESS(10, int, INT_MAX, add),
    TEST_SUCCESS( 0, int, 0,       sub),
    TEST_SUCCESS( 1, int, 0,       sub),
    TEST_SUCCESS( 2, int, 0,       sub),
    TEST_SUCCESS( 3, int, INT_MIN, sub),
    TEST_SUCCESS( 4, int, INT_MIN, sub),
    TEST_SUCCESS( 5, int, INT_MAX, sub),
    TEST_SUCCESS( 6, int, INT_MAX, sub),
    TEST_SUCCESS( 0, int, 0, mul),
    TEST_SUCCESS( 1, int, 1, mul),
    TEST_SUCCESS( 2, int, 0, mul),
    TEST_SUCCESS( 3, int, 0, mul),
    TEST_SUCCESS( 4, int, INT_MIN, mul),
    TEST_SUCCESS( 5, int, INT_MIN, mul),
    TEST_SUCCESS( 6, int, 0, mul),
    TEST_SUCCESS( 7, int, 0, mul),
    TEST_SUCCESS( 8, int, INT_MAX, mul),
    TEST_SUCCESS( 9, int, INT_MAX, mul),
    TEST_SUCCESS(10, int, INT_MAX, mul),
    TEST_SUCCESS(11, int, INT_MAX, mul),
    TEST_SUCCESS(12, int, NEG_INT_MAX, mul),
    TEST_SUCCESS(13, int, NEG_INT_MAX, mul),
    TEST_SUCCESS( 0, int, 0,       div),
    TEST_SUCCESS( 1, int, 1,       div),
    TEST_SUCCESS( 2, int, 1,       div),
    TEST_SUCCESS( 3, int, INT_MIN, div),
    TEST_SUCCESS( 4, int, 1,       div),
    TEST_SUCCESS( 5, int, INT_MAX, div),
    TEST_E_ERRNO( 0, int, ERANGE, add),
    TEST_E_ERRNO( 1, int, ERANGE, sub),
    TEST_E_ERRNO( 2, int, ERANGE, mul),
    TEST_E_ERRNO( 3, int, ERANGE, mul),
    TEST_E_ERRNO( 4, int, ERANGE, mul),
    TEST_E_ERRNO( 5, int, EDOM,   div),
    TEST_E_ERRNO( 6, int, ERANGE, div),
    /* 'long' */
    TEST_SUCCESS( 0, long, 0,        add),
    TEST_SUCCESS( 1, long, 0,        add),
    TEST_SUCCESS( 2, long, 0,        add),
    TEST_SUCCESS( 3, long, LONG_MIN, add),
    TEST_SUCCESS( 4, long, LONG_MIN, add),
    TEST_SUCCESS( 5, long, LONG_MIN, add),
    TEST_SUCCESS( 6, long, LONG_MIN, add),
    TEST_SUCCESS( 7, long, LONG_MAX, add),
    TEST_SUCCESS( 8, long, LONG_MAX, add),
    TEST_SUCCESS( 9, long, LONG_MAX, add),
    TEST_SUCCESS(10, long, LONG_MAX, add),
    TEST_SUCCESS( 0, long, 0,        sub),
    TEST_SUCCESS( 1, long, 0,        sub),
    TEST_SUCCESS( 2, long, 0,        sub),
    TEST_SUCCESS( 3, long, LONG_MIN, sub),
    TEST_SUCCESS( 4, long, LONG_MIN, sub),
    TEST_SUCCESS( 5, long, LONG_MAX, sub),
    TEST_SUCCESS( 6, long, LONG_MAX, sub),
    TEST_SUCCESS( 0, long, 0, mul),
    TEST_SUCCESS( 1, long, 1, mul),
    TEST_SUCCESS( 2, long, 0, mul),
    TEST_SUCCESS( 3, long, 0, mul),
    TEST_SUCCESS( 4, long, LONG_MIN, mul),
    TEST_SUCCESS( 5, long, LONG_MIN, mul),
    TEST_SUCCESS( 6, long, 0, mul),
    TEST_SUCCESS( 7, long, 0, mul),
    TEST_SUCCESS( 8, long, LONG_MAX, mul),
    TEST_SUCCESS( 9, long, LONG_MAX, mul),
    TEST_SUCCESS(10, long, LONG_MAX, mul),
    TEST_SUCCESS(11, long, LONG_MAX, mul),
    TEST_SUCCESS(12, long, NEG_LONG_MAX, mul),
    TEST_SUCCESS(13, long, NEG_LONG_MAX, mul),
    TEST_SUCCESS( 0, long, 0,        div),
    TEST_SUCCESS( 1, long, 1,        div),
    TEST_SUCCESS( 2, long, 1,        div),
    TEST_SUCCESS( 3, long, LONG_MIN, div),
    TEST_SUCCESS( 4, long, 1,        div),
    TEST_SUCCESS( 5, long, LONG_MAX, div),
    TEST_E_ERRNO( 0, long, ERANGE, add),
    TEST_E_ERRNO( 1, long, ERANGE, sub),
    TEST_E_ERRNO( 2, long, ERANGE, mul),
    TEST_E_ERRNO( 3, long, ERANGE, mul),
    TEST_E_ERRNO( 4, long, ERANGE, mul),
    TEST_E_ERRNO( 5, long, EDOM,   div),
    TEST_E_ERRNO( 6, long, ERANGE, div),
    /* 'long long' */
    TEST_SUCCESS( 0, llong, 0,         add),
    TEST_SUCCESS( 1, llong, 0,         add),
    TEST_SUCCESS( 2, llong, 0,         add),
    TEST_SUCCESS( 3, llong, LLONG_MIN, add),
    TEST_SUCCESS( 4, llong, LLONG_MIN, add),
    TEST_SUCCESS( 5, llong, LLONG_MIN, add),
    TEST_SUCCESS( 6, llong, LLONG_MIN, add),
    TEST_SUCCESS( 7, llong, LLONG_MAX, add),
    TEST_SUCCESS( 8, llong, LLONG_MAX, add),
    TEST_SUCCESS( 9, llong, LLONG_MAX, add),
    TEST_SUCCESS(10, llong, LLONG_MAX, add),
    TEST_SUCCESS( 0, llong, 0,         sub),
    TEST_SUCCESS( 1, llong, 0,         sub),
    TEST_SUCCESS( 2, llong, 0,         sub),
    TEST_SUCCESS( 3, llong, LLONG_MIN, sub),
    TEST_SUCCESS( 4, llong, LLONG_MIN, sub),
    TEST_SUCCESS( 5, llong, LLONG_MAX, sub),
    TEST_SUCCESS( 6, llong, LLONG_MAX, sub),
    TEST_SUCCESS( 0, llong, 0, mul),
    TEST_SUCCESS( 1, llong, 1, mul),
    TEST_SUCCESS( 2, llong, 0, mul),
    TEST_SUCCESS( 3, llong, 0, mul),
    TEST_SUCCESS( 4, llong, LLONG_MIN, mul),
    TEST_SUCCESS( 5, llong, LLONG_MIN, mul),
    TEST_SUCCESS( 6, llong, 0, mul),
    TEST_SUCCESS( 7, llong, 0, mul),
    TEST_SUCCESS( 8, llong, LLONG_MAX, mul),
    TEST_SUCCESS( 9, llong, LLONG_MAX, mul),
    TEST_SUCCESS(10, llong, LLONG_MAX, mul),
    TEST_SUCCESS(11, llong, LLONG_MAX, mul),
    TEST_SUCCESS(12, llong, NEG_LLONG_MAX, mul),
    TEST_SUCCESS(13, llong, NEG_LLONG_MAX, mul),
    TEST_SUCCESS( 0, llong, 0,         div),
    TEST_SUCCESS( 1, llong, 1,         div),
    TEST_SUCCESS( 2, llong, 1,         div),
    TEST_SUCCESS( 3, llong, LLONG_MIN, div),
    TEST_SUCCESS( 4, llong, 1,         div),
    TEST_SUCCESS( 5, llong, LLONG_MAX, div),
    TEST_E_ERRNO( 0, llong, ERANGE, add),
    TEST_E_ERRNO( 1, llong, ERANGE, sub),
    TEST_E_ERRNO( 2, llong, ERANGE, mul),
    TEST_E_ERRNO( 3, llong, ERANGE, mul),
    TEST_E_ERRNO( 4, llong, ERANGE, mul),
    TEST_E_ERRNO( 5, llong, EDOM,   div),
    TEST_E_ERRNO( 6, llong, ERANGE, div),
    /* 'unsigned char' */
    TEST_SUCCESS(0, uchar, 0,         add),
    TEST_SUCCESS(1, uchar, UCHAR_MAX, add),
    TEST_SUCCESS(2, uchar, UCHAR_MAX, add),
    TEST_SUCCESS(3, uchar, UCHAR_MAX, add),
    TEST_SUCCESS(4, uchar, UCHAR_MAX, add),
    TEST_SUCCESS(0, uchar, 0,         sub),
    TEST_SUCCESS(1, uchar, 1,         sub),
    TEST_SUCCESS(2, uchar, UCHAR_MAX, sub),
    TEST_SUCCESS(0, uchar, 0,         mul),
    TEST_SUCCESS(1, uchar, 1,         mul),
    TEST_SUCCESS(2, uchar, 0,         mul),
    TEST_SUCCESS(3, uchar, 0,         mul),
    TEST_SUCCESS(4, uchar, UCHAR_MAX, mul),
    TEST_SUCCESS(5, uchar, UCHAR_MAX, mul),
    TEST_SUCCESS(0, uchar, 0,         div),
    TEST_SUCCESS(1, uchar, 1,         div),
    TEST_SUCCESS(2, uchar, 1,         div),
    TEST_SUCCESS(3, uchar, UCHAR_MAX, div),
    TEST_E_ERRNO(0, uchar, ERANGE, add),
    TEST_E_ERRNO(1, uchar, ERANGE, sub),
    TEST_E_ERRNO(2, uchar, ERANGE, mul),
    TEST_E_ERRNO(3, uchar, EDOM,   div),
    /* 'unsigned short' */
    TEST_SUCCESS(0, ushort, 0,         add),
    TEST_SUCCESS(1, ushort, USHRT_MAX, add),
    TEST_SUCCESS(2, ushort, USHRT_MAX, add),
    TEST_SUCCESS(3, ushort, USHRT_MAX, add),
    TEST_SUCCESS(4, ushort, USHRT_MAX, add),
    TEST_SUCCESS(0, ushort, 0,         sub),
    TEST_SUCCESS(1, ushort, 1,         sub),
    TEST_SUCCESS(2, ushort, USHRT_MAX, sub),
    TEST_SUCCESS(0, ushort, 0,         mul),
    TEST_SUCCESS(1, ushort, 1,         mul),
    TEST_SUCCESS(2, ushort, 0,         mul),
    TEST_SUCCESS(3, ushort, 0,         mul),
    TEST_SUCCESS(4, ushort, USHRT_MAX, mul),
    TEST_SUCCESS(5, ushort, USHRT_MAX, mul),
    TEST_SUCCESS(0, ushort, 0,         div),
    TEST_SUCCESS(1, ushort, 1,         div),
    TEST_SUCCESS(2, ushort, 1,         div),
    TEST_SUCCESS(3, ushort, USHRT_MAX, div),
    TEST_E_ERRNO(0, ushort, ERANGE, add),
    TEST_E_ERRNO(1, ushort, ERANGE, sub),
    TEST_E_ERRNO(2, ushort, ERANGE, mul),
    TEST_E_ERRNO(3, ushort, EDOM,   div),
    /* 'unsigned int' */
    TEST_SUCCESS(0, uint, 0,        add),
    TEST_SUCCESS(1, uint, UINT_MAX, add),
    TEST_SUCCESS(2, uint, UINT_MAX, add),
    TEST_SUCCESS(3, uint, UINT_MAX, add),
    TEST_SUCCESS(4, uint, UINT_MAX, add),
    TEST_SUCCESS(0, uint, 0,        sub),
    TEST_SUCCESS(1, uint, 1,        sub),
    TEST_SUCCESS(2, uint, UINT_MAX, sub),
    TEST_SUCCESS(0, uint, 0,        mul),
    TEST_SUCCESS(1, uint, 1,        mul),
    TEST_SUCCESS(2, uint, 0,        mul),
    TEST_SUCCESS(3, uint, 0,        mul),
    TEST_SUCCESS(4, uint, UINT_MAX, mul),
    TEST_SUCCESS(5, uint, UINT_MAX, mul),
    TEST_SUCCESS(0, uint, 0,        div),
    TEST_SUCCESS(1, uint, 1,        div),
    TEST_SUCCESS(2, uint, 1,        div),
    TEST_SUCCESS(3, uint, UINT_MAX, div),
    TEST_E_ERRNO(0, uint, ERANGE, add),
    TEST_E_ERRNO(1, uint, ERANGE, sub),
    TEST_E_ERRNO(2, uint, ERANGE, mul),
    TEST_E_ERRNO(3, uint, EDOM,   div),
    /* 'unsigned long' */
    TEST_SUCCESS(0, ulong, 0,         add),
    TEST_SUCCESS(1, ulong, ULONG_MAX, add),
    TEST_SUCCESS(2, ulong, ULONG_MAX, add),
    TEST_SUCCESS(3, ulong, ULONG_MAX, add),
    TEST_SUCCESS(4, ulong, ULONG_MAX, add),
    TEST_SUCCESS(0, ulong, 0,         sub),
    TEST_SUCCESS(1, ulong, 1,         sub),
    TEST_SUCCESS(2, ulong, ULONG_MAX, sub),
    TEST_SUCCESS(0, ulong, 0,         mul),
    TEST_SUCCESS(1, ulong, 1,         mul),
    TEST_SUCCESS(2, ulong, 0,         mul),
    TEST_SUCCESS(3, ulong, 0,         mul),
    TEST_SUCCESS(4, ulong, ULONG_MAX, mul),
    TEST_SUCCESS(5, ulong, ULONG_MAX, mul),
    TEST_SUCCESS(0, ulong, 0,         div),
    TEST_SUCCESS(1, ulong, 1,         div),
    TEST_SUCCESS(2, ulong, 1,         div),
    TEST_SUCCESS(3, ulong, ULONG_MAX, div),
    TEST_E_ERRNO(0, ulong, ERANGE, add),
    TEST_E_ERRNO(1, ulong, ERANGE, sub),
    TEST_E_ERRNO(2, ulong, ERANGE, mul),
    TEST_E_ERRNO(3, ulong, EDOM,   div),
    /* 'unsigned long long' */
    TEST_SUCCESS(0, ullong, 0,          add),
    TEST_SUCCESS(1, ullong, ULLONG_MAX, add),
    TEST_SUCCESS(2, ullong, ULLONG_MAX, add),
    TEST_SUCCESS(3, ullong, ULLONG_MAX, add),
    TEST_SUCCESS(4, ullong, ULLONG_MAX, add),
    TEST_SUCCESS(0, ullong, 0,          sub),
    TEST_SUCCESS(1, ullong, 1,          sub),
    TEST_SUCCESS(2, ullong, ULLONG_MAX, sub),
    TEST_SUCCESS(0, ullong, 0,          mul),
    TEST_SUCCESS(1, ullong, 1,          mul),
    TEST_SUCCESS(2, ullong, 0,          mul),
    TEST_SUCCESS(3, ullong, 0,          mul),
    TEST_SUCCESS(4, ullong, ULLONG_MAX, mul),
    TEST_SUCCESS(5, ullong, ULLONG_MAX, mul),
    TEST_SUCCESS(0, ullong, 0,          div),
    TEST_SUCCESS(1, ullong, 1,          div),
    TEST_SUCCESS(2, ullong, 1,          div),
    TEST_SUCCESS(3, ullong, ULLONG_MAX, div),
    TEST_E_ERRNO(0, ullong, ERANGE, add),
    TEST_E_ERRNO(1, ullong, ERANGE, sub),
    TEST_E_ERRNO(2, ullong, ERANGE, mul),
    TEST_E_ERRNO(3, ullong, EDOM,   div),
    /* 'float' */
    TEST_SUCCESS( 0, float, 0,           add),
    TEST_SUCCESS( 1, float, 0,           add),
    TEST_SUCCESS( 2, float, 0,           add),
    TEST_SUCCESS( 3, float, NEG_FLT_EXT, add),
    TEST_SUCCESS( 4, float, NEG_FLT_EXT, add),
    TEST_SUCCESS( 5, float, NEG_TWO,     add),
    TEST_SUCCESS( 6, float, POS_FLT_EXT, add),
    TEST_SUCCESS( 7, float, POS_FLT_EXT, add),
    TEST_SUCCESS( 8, float, 2,           add),
    TEST_SUCCESS( 0, float, 0,           sub),
    TEST_SUCCESS( 1, float, 0,           sub),
    TEST_SUCCESS( 2, float, 0,           sub),
    TEST_SUCCESS( 3, float, NEG_FLT_EXT, sub),
    TEST_SUCCESS( 4, float, NEG_FLT_EXT, sub),
    TEST_SUCCESS( 5, float, POS_FLT_EXT, sub),
    TEST_SUCCESS( 6, float, POS_FLT_EXT, sub),
    TEST_SUCCESS( 0, float, 0,           mul),
    TEST_SUCCESS( 1, float, 1,           mul),
    TEST_SUCCESS( 2, float, 0,           mul),
    TEST_SUCCESS( 3, float, 0,           mul),
    TEST_SUCCESS( 4, float, NEG_FLT_EXT, mul),
    TEST_SUCCESS( 5, float, NEG_FLT_EXT, mul),
    TEST_SUCCESS( 6, float, 0,           mul),
    TEST_SUCCESS( 7, float, 0,           mul),
    TEST_SUCCESS( 8, float, POS_FLT_EXT, mul),
    TEST_SUCCESS( 9, float, POS_FLT_EXT, mul),
    TEST_SUCCESS(10, float, POS_FLT_EXT, mul),
    TEST_SUCCESS(11, float, POS_FLT_EXT, mul),
    TEST_SUCCESS(12, float, NEG_FLT_EXT, mul),
    TEST_SUCCESS(13, float, NEG_FLT_EXT, mul),
    TEST_SUCCESS( 0, float, 0,           div),
    TEST_SUCCESS( 1, float, 1,           div),
    TEST_SUCCESS( 2, float, 1,           div),
    TEST_SUCCESS( 3, float, NEG_FLT_EXT, div),
    TEST_SUCCESS( 4, float, 1,           div),
    TEST_SUCCESS( 5, float, POS_FLT_EXT, div),
    TEST_E_ERRNO( 0, float, ERANGE, add),
    TEST_E_ERRNO( 1, float, ERANGE, sub),
    TEST_E_ERRNO( 2, float, ERANGE, mul),
    TEST_E_ERRNO( 3, float, ERANGE, mul),
    TEST_E_ERRNO( 4, float, ERANGE, mul),
    TEST_E_ERRNO( 5, float, ERANGE, mul),
    TEST_E_ERRNO( 6, float, EDOM,   div),
    TEST_E_ERRNO( 7, float, ERANGE, div),
    TEST_E_ERRNO( 8, float, ERANGE, div),
    TEST_E_ERRNO( 9, float, ERANGE, div),
    TEST_E_ERRNO(10, float, ERANGE, div),
    /* 'double' */
    TEST_SUCCESS( 0, double, 0,           add),
    TEST_SUCCESS( 1, double, 0,           add),
    TEST_SUCCESS( 2, double, 0,           add),
    TEST_SUCCESS( 3, double, NEG_DBL_EXT, add),
    TEST_SUCCESS( 4, double, NEG_DBL_EXT, add),
    TEST_SUCCESS( 5, double, NEG_TWO,     add),
    TEST_SUCCESS( 6, double, POS_DBL_EXT, add),
    TEST_SUCCESS( 7, double, POS_DBL_EXT, add),
    TEST_SUCCESS( 8, double, 2,           add),
    TEST_SUCCESS( 0, double, 0,           sub),
    TEST_SUCCESS( 1, double, 0,           sub),
    TEST_SUCCESS( 2, double, 0,           sub),
    TEST_SUCCESS( 3, double, NEG_DBL_EXT, sub),
    TEST_SUCCESS( 4, double, NEG_DBL_EXT, sub),
    TEST_SUCCESS( 5, double, POS_DBL_EXT, sub),
    TEST_SUCCESS( 6, double, POS_DBL_EXT, sub),
    TEST_SUCCESS( 0, double, 0,           mul),
    TEST_SUCCESS( 1, double, 1,           mul),
    TEST_SUCCESS( 2, double, 0,           mul),
    TEST_SUCCESS( 3, double, 0,           mul),
    TEST_SUCCESS( 4, double, NEG_DBL_EXT, mul),
    TEST_SUCCESS( 5, double, NEG_DBL_EXT, mul),
    TEST_SUCCESS( 6, double, 0,           mul),
    TEST_SUCCESS( 7, double, 0,           mul),
    TEST_SUCCESS( 8, double, POS_DBL_EXT, mul),
    TEST_SUCCESS( 9, double, POS_DBL_EXT, mul),
    TEST_SUCCESS(10, double, POS_DBL_EXT, mul),
    TEST_SUCCESS(11, double, POS_DBL_EXT, mul),
    TEST_SUCCESS(12, double, NEG_DBL_EXT, mul),
    TEST_SUCCESS(13, double, NEG_DBL_EXT, mul),
    TEST_SUCCESS( 0, double, 0,           div),
    TEST_SUCCESS( 1, double, 1,           div),
    TEST_SUCCESS( 2, double, 1,           div),
    TEST_SUCCESS( 3, double, NEG_DBL_EXT, div),
    TEST_SUCCESS( 4, double, 1,           div),
    TEST_SUCCESS( 5, double, POS_DBL_EXT, div),
    TEST_E_ERRNO( 0, double, ERANGE, add),
    TEST_E_ERRNO( 1, double, ERANGE, sub),
    TEST_E_ERRNO( 2, double, ERANGE, mul),
    TEST_E_ERRNO( 3, double, ERANGE, mul),
    TEST_E_ERRNO( 4, double, ERANGE, mul),
    TEST_E_ERRNO( 5, double, ERANGE, mul),
    TEST_E_ERRNO( 6, double, EDOM,   div),
    TEST_E_ERRNO( 7, double, ERANGE, div),
    TEST_E_ERRNO( 8, double, ERANGE, div),
    TEST_E_ERRNO( 9, double, ERANGE, div),
    TEST_E_ERRNO(10, double, ERANGE, div),
    /* 'long double' */
    TEST_SUCCESS( 0, ldouble, 0,            add),
    TEST_SUCCESS( 1, ldouble, 0,            add),
    TEST_SUCCESS( 2, ldouble, 0,            add),
    TEST_SUCCESS( 3, ldouble, NEG_LDBL_EXT, add),
    TEST_SUCCESS( 4, ldouble, NEG_LDBL_EXT, add),
    TEST_SUCCESS( 5, ldouble, NEG_TWO,      add),
    TEST_SUCCESS( 6, ldouble, POS_LDBL_EXT, add),
    TEST_SUCCESS( 7, ldouble, POS_LDBL_EXT, add),
    TEST_SUCCESS( 8, ldouble, 2,            add),
    TEST_SUCCESS( 0, ldouble, 0,            sub),
    TEST_SUCCESS( 1, ldouble, 0,            sub),
    TEST_SUCCESS( 2, ldouble, 0,            sub),
    TEST_SUCCESS( 3, ldouble, NEG_LDBL_EXT, sub),
    TEST_SUCCESS( 4, ldouble, NEG_LDBL_EXT, sub),
    TEST_SUCCESS( 5, ldouble, POS_LDBL_EXT, sub),
    TEST_SUCCESS( 6, ldouble, POS_LDBL_EXT, sub),
    TEST_SUCCESS( 0, ldouble, 0,            mul),
    TEST_SUCCESS( 1, ldouble, 1,            mul),
    TEST_SUCCESS( 2, ldouble, 0,            mul),
    TEST_SUCCESS( 3, ldouble, 0,            mul),
    TEST_SUCCESS( 4, ldouble, NEG_LDBL_EXT, mul),
    TEST_SUCCESS( 5, ldouble, NEG_LDBL_EXT, mul),
    TEST_SUCCESS( 6, ldouble, 0,            mul),
    TEST_SUCCESS( 7, ldouble, 0,            mul),
    TEST_SUCCESS( 8, ldouble, POS_LDBL_EXT, mul),
    TEST_SUCCESS( 9, ldouble, POS_LDBL_EXT, mul),
    TEST_SUCCESS(10, ldouble, POS_LDBL_EXT, mul),
    TEST_SUCCESS(11, ldouble, POS_LDBL_EXT, mul),
    TEST_SUCCESS(12, ldouble, NEG_LDBL_EXT, mul),
    TEST_SUCCESS(13, ldouble, NEG_LDBL_EXT, mul),
    TEST_SUCCESS( 0, ldouble, 0,            div),
    TEST_SUCCESS( 1, ldouble, 1,            div),
    TEST_SUCCESS( 2, ldouble, 1,            div),
    TEST_SUCCESS( 3, ldouble, NEG_LDBL_EXT, div),
    TEST_SUCCESS( 4, ldouble, 1,            div),
    TEST_SUCCESS( 5, ldouble, POS_LDBL_EXT, div),
    TEST_E_ERRNO( 0, ldouble, ERANGE, add),
    TEST_E_ERRNO( 1, ldouble, ERANGE, sub),
    TEST_E_ERRNO( 2, ldouble, ERANGE, mul),
    TEST_E_ERRNO( 3, ldouble, ERANGE, mul),
    TEST_E_ERRNO( 4, ldouble, ERANGE, mul),
    TEST_E_ERRNO( 5, ldouble, ERANGE, mul),
    TEST_E_ERRNO( 6, ldouble, EDOM,   div),
    TEST_E_ERRNO( 7, ldouble, ERANGE, div),
    TEST_E_ERRNO( 8, ldouble, ERANGE, div),
    TEST_E_ERRNO( 9, ldouble, ERANGE, div),
    TEST_E_ERRNO(10, ldouble, ERANGE, div)
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
