/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <stdarg.h>

/*
 * TAP output with different levels of verbosity
 */

enum tap_verbosity {
    TAP_ERROR,
    TAP_INFO,
    TAP_DEBUG
};

extern enum tap_verbosity g_tap_verbosity; /* default is TAP_ERROR */

void
tap_error_va(const char* msg, va_list ap);

void
tap_info_va(const char* msg, va_list ap);

void
tap_debug_va(const char* msg, va_list ap);

void
tap_error(const char* msg, ...);

void
tap_info(const char* msg, ...);

void
tap_debug(const char* msg, ...);

/*
 * Error reporting
 */

void
tap_error_errno_at(const char* function, int errnum, const char* filename,
                   unsigned long line);

#define tap_error_errno(function, errnum)   \
    tap_error_errno_at((function), (errnum), __FILE__, __LINE__)
