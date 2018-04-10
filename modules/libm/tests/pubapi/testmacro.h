/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "picotm/picotm.h"
#include "picotm/picotm-module.h"
#include "safeblk.h"
#include "taputils.h"

#define __TEST_SYMBOL(_func)    \
    test_ ## _func

/*
 * Test for success
 */

#define __TEST_SUCCESS_SYMBOL(_func)    \
    __TEST_SYMBOL(success_ ## _func)

#define __TEST_SUCCESS(_sym, _cond, _rtype, _func, ...)                     \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        if (!(_cond)) {                                                     \
            tap_info("skipping next test for " #_func "_tx();"              \
                     " condition failed");                                  \
            return;                                                         \
        }                                                                   \
                                                                            \
        picotm_safe const _rtype value = _func(__VA_ARGS__);                \
                                                                            \
        picotm_begin                                                        \
                                                                            \
            _rtype tx_value = _func ## _tx(__VA_ARGS__);                    \
                                                                            \
            if (tx_value != value) {                                        \
                tap_error("mismatching results for functions "              \
                          #_func "() and " #_func "_tx()"                   \
                          " expected '%f', got '%f'\n",                     \
                          (double)value, (double)tx_value);                 \
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;       \
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);  \
                picotm_error_mark_as_non_recoverable(&error);               \
                picotm_recover_from_error(&error);                          \
            }                                                               \
                                                                            \
        picotm_commit                                                       \
            abort_transaction_on_error(__func__);                           \
        picotm_end                                                          \
    }

#define TEST_SUCCESS_IF(_cond, _rtype, _func, ...)                      \
    __TEST_SUCCESS(__TEST_SUCCESS_SYMBOL(_func), _cond, _rtype, _func,  \
                   __VA_ARGS__)

#define TEST_SUCCESS(_rtype, _func, ...)                            \
    __TEST_SUCCESS(__TEST_SUCCESS_SYMBOL(_func), 1, _rtype, _func,  \
                   __VA_ARGS__)

#define __TEST_SUCCESS_NAN(_sym, _cond, _rtype, _func, ...)                 \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        if (!(_cond)) {                                                     \
            tap_info("skipping next test for " #_func "_tx();"              \
                     " condition failed");                                  \
            return;                                                         \
        }                                                                   \
                                                                            \
        picotm_safe const _rtype value = _func(__VA_ARGS__);                \
                                                                            \
        picotm_begin                                                        \
                                                                            \
            _rtype tx_value = _func(__VA_ARGS__);                           \
                                                                            \
            if (!isnan(tx_value) || !isnan(value)) {                        \
                tap_error("mismatching results for functions "              \
                          #_func "() and " #_func "_tx()"                   \
                          " expected 'nan/nan', got '%f/%f'\n",             \
                          (double)value, (double)tx_value);                 \
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;       \
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);  \
                picotm_error_mark_as_non_recoverable(&error);               \
                picotm_recover_from_error(&error);                          \
            }                                                               \
                                                                            \
        picotm_commit                                                       \
            abort_transaction_on_error(__func__);                           \
        picotm_end                                                          \
    }

#define TEST_SUCCESS_NAN_IF(_cond, _rtype, _func, ...)                      \
    __TEST_SUCCESS_NAN(__TEST_SUCCESS_SYMBOL(_func), _cond, _rtype, _func,  \
                       __VA_ARGS__)

#define TEST_SUCCESS_NAN(_rtype, _func, ...)                            \
    __TEST_SUCCESS_NAN(__TEST_SUCCESS_SYMBOL(_func), 1, _rtype, _func,  \
                       __VA_ARGS__)

#define __TEST_SUCCESS_P(_sym, _cond, _rtype, _func, _ptype, ...)           \
    static void                                                             \
    _sym(unsigned int tid)                                                  \
    {                                                                       \
        if (!(_cond)) {                                                     \
            tap_info("skipping next test for " #_func "_tx();"              \
                     " condition failed");                                  \
            return;                                                         \
        }                                                                   \
                                                                            \
        _ptype pvalue;                                                      \
        picotm_safe const _rtype value = _func(__VA_ARGS__, &pvalue);       \
                                                                            \
        picotm_begin                                                        \
                                                                            \
            _ptype tx_pvalue;                                               \
            _rtype tx_value = _func(__VA_ARGS__, &tx_pvalue);               \
                                                                            \
            if (tx_value != value) {                                        \
                tap_error("mismatching results for functions "              \
                          #_func "() and " #_func "_tx()"                   \
                          " expected '%f', got '%f'\n",                     \
                          (double)value, (double)tx_value);                 \
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;       \
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);  \
                picotm_error_mark_as_non_recoverable(&error);               \
                picotm_recover_from_error(&error);                          \
            }                                                               \
            if (tx_pvalue != pvalue) {                                      \
                tap_error("mismatching results for functions "              \
                          #_func "() and " #_func "_tx()"                   \
                          " expected '%f', got '%f'\n",                     \
                          (double)pvalue, (double)tx_pvalue);               \
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;       \
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);  \
                picotm_error_mark_as_non_recoverable(&error);               \
                picotm_recover_from_error(&error);                          \
            }                                                               \
                                                                            \
        picotm_commit                                                       \
            abort_transaction_on_error(__func__);                           \
        picotm_end                                                          \
    }

#define TEST_SUCCESS_P_IF(_cond, _rtype, _func, _ptype, ...)                \
    __TEST_SUCCESS_P(__TEST_SUCCESS_SYMBOL(_func), _cond, _rtype, _func,    \
                     _ptype, __VA_ARGS__)

#define TEST_SUCCESS_P(_rtype, _func, _ptype, ...)                      \
    __TEST_SUCCESS_P(__TEST_SUCCESS_SYMBOL(_func), 1, _rtype, _func,    \
                     _ptype, __VA_ARGS__)

#define TEST_SUCCESS_SKIP(_func)                        \
    static void                                         \
    __TEST_SUCCESS_SYMBOL(_func)(unsigned int tid)      \
    {                                                   \
        tap_info("skipping test for " #_func "_tx()");  \
    }

#define TEST_SUCCESS_FUNC(_func) \
    {"Test " #_func "_tx()", __TEST_SUCCESS_SYMBOL(_func), NULL, NULL}

/*
 * Test for errno code
 */

#define __TEST_E_ERRNO_SYMBOL(_errno, _func)    \
    __TEST_SYMBOL(e_ ## _errno ## _ ## _func)

#define __TEST_E_ERRNO(_sym, _cond, _errno, _func, ...)                 \
    static void                                                         \
    _sym(unsigned int tid)                                              \
    {                                                                   \
        if (!(_cond)) {                                                 \
            tap_info("skipping next test for " #_func "_tx();"          \
                     " condition failed");                              \
            return;                                                     \
        }                                                               \
                                                                        \
        bool error_detected = false;                                    \
                                                                        \
        errno = 0;                                                      \
                                                                        \
        picotm_begin                                                    \
                                                                        \
            _func ## _tx(__VA_ARGS__);                                  \
                                                                        \
        picotm_commit                                                   \
                                                                        \
            if ((picotm_error_status() == PICOTM_ERRNO) &&              \
                picotm_error_as_errno() == (_errno)) {                  \
                error_detected = true;                                  \
            }                                                           \
                                                                        \
        picotm_end                                                      \
                                                                        \
        if (!error_detected) {                                          \
            tap_error("%s, No error detected.", __func__);              \
            abort_safe_block();                                         \
        }                                                               \
    }

#define TEST_E_ERRNO_IF(_cond, _errno, _func, ...)                      \
    __TEST_E_ERRNO(__TEST_E_ERRNO_SYMBOL(errno_ ## _errno, _func),      \
                   _cond, _errno, _func, __VA_ARGS__)

#define TEST_E_ERRNO(_errno, _func, ...)                                \
    __TEST_E_ERRNO(__TEST_E_ERRNO_SYMBOL(errno_ ## _errno, _func),      \
                   1, _errno, _func, __VA_ARGS__)

#define TEST_E_ERRNO_FUNC(_errno, _func)                            \
    {"Test " #_func "_tx() for " #_errno,                           \
        __TEST_E_ERRNO_SYMBOL(errno_ ## _errno, _func), NULL, NULL}

/*
 * Test for floating-point exception
 */

int
errno_of_flag(int flag);

#define __TEST_E_EXCPT_SYMBOL(_flag, _func)     \
    __TEST_SYMBOL(e_ ## _flag ## _ ## _func)

#define __TEST_E_EXCPT(_sym, _cond, _flag, _func, ...)              \
    static void                                                     \
    _sym(unsigned int tid)                                          \
    {                                                               \
        if (!(_cond)) {                                             \
            tap_info("skipping next test for " #_func "_tx();"      \
                     " condition failed");                          \
            return;                                                 \
        }                                                           \
                                                                    \
        bool error_detected = false;                                \
                                                                    \
        errno = 0;                                                  \
        feclearexcept(FE_ALL_EXCEPT);                               \
                                                                    \
        picotm_begin                                                \
                                                                    \
            _func ## _tx(__VA_ARGS__);                              \
                                                                    \
        picotm_commit                                               \
                                                                    \
            if ((picotm_error_status() == PICOTM_ERRNO) &&          \
                picotm_error_as_errno() == errno_of_flag(_flag)) {  \
                error_detected = true;                              \
            }                                                       \
                                                                    \
        picotm_end                                                  \
                                                                    \
        if (!error_detected) {                                      \
            tap_error("%s, No error detected.", __func__);          \
            abort_safe_block();                                     \
        }                                                           \
    }

#define TEST_E_EXCPT_IF(_cond, _flag, _func, ...)                   \
    __TEST_E_EXCPT(__TEST_E_EXCPT_SYMBOL(excpt_ ## _flag, _func),   \
                   _cond, _flag, _func, __VA_ARGS__)

#define TEST_E_EXCPT(_flag, _func, ...)                             \
    __TEST_E_EXCPT(__TEST_E_EXCPT_SYMBOL(excpt_ ## _flag, _func),   \
                   1, _flag, _func, __VA_ARGS__)

#define TEST_E_EXCPT_FUNC(_flag, _func)                             \
    {"Test " #_func "_tx() for " #_flag,                            \
        __TEST_E_EXCPT_SYMBOL(excpt_ ## _flag, _func), NULL, NULL}
