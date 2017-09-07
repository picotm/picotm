/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#pragma once

#include <stdarg.h>

void
tap_version(unsigned long version);

void
tap_plan(unsigned long n);

/*
 * Interfaces with variable number of arguments
 */

void
tap_ok(unsigned long testnum, const char* msg, ...);

void
tap_ok_todo(unsigned long testnum, const char* msg, ...);

void
tap_ok_skip(unsigned long testnum, const char* msg, ...);

void
tap_not_ok(unsigned long testnum, const char* msg, ...);

void
tap_not_ok_todo(unsigned long testnum, const char* msg, ...);

void
tap_not_ok_skip(unsigned long testnum, const char* msg, ...);

void
tap_bail_out(const char* desc, ...);

void
tap_diag(const char* msg, ...);

/*
 * Interfaces with va_list argument
 */

void
tap_ok_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_ok_todo_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_ok_skip_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_not_ok_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_not_ok_todo_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_not_ok_skip_va(unsigned long testnum, const char* msg, va_list ap);

void
tap_bail_out_va(const char* desc, va_list ap);

void
tap_diag_va(const char* msg, va_list ap);
