/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include "taputils.h"

#define __TEST_SYMBOL(_func)    \
    test_ ## _func

#define __TEST_SUCCESS_SYMBOL(_func)    \
    __TEST_SYMBOL(success_ ## _func)

#define TEST_SUCCESS(_rtype, _func, ...)                                    \
    static void                                                             \
    __TEST_SUCCESS_SYMBOL(_func)(unsigned int tid)                          \
    {                                                                       \
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

#define TEST_SUCCESS_NAN(_rtype, _func, ...)                                \
    static void                                                             \
    __TEST_SUCCESS_SYMBOL(_func)(unsigned int tid)                          \
    {                                                                       \
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

#define TEST_SUCCESS_P(_rtype, _func, _ptype, ...)                          \
    static void                                                             \
    __TEST_SUCCESS_SYMBOL(_func)(unsigned int tid)                          \
    {                                                                       \
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

#define TEST_SUCCESS_SKIP(_func)                        \
    static void                                         \
    __TEST_SUCCESS_SYMBOL(_func)(unsigned int tid)      \
    {                                                   \
        tap_info("skipping test for " #_func "_tx()");  \
    }

#define TEST_SUCCESS_FUNC(_func) \
    {"Test " #_func "_tx()", __TEST_SUCCESS_SYMBOL(_func), NULL, NULL}

