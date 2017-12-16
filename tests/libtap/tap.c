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
 */

#include "tap.h"
#include <stdio.h>
#include <stdlib.h>

static void
safe_vprintf(const char* format, va_list ap)
{
    int res = vprintf(format, ap);
    if (res < 0) {
        perror("vprintf()");
        abort();
    }
}

static void
safe_printf(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    safe_vprintf(format, ap);
    va_end(ap);
}

void
tap_version(unsigned long version)
{
    if (version > 12) {
        safe_printf("TAP version %lu\n", version);
    }
}

void
tap_plan(unsigned long n)
{
    safe_printf("1..%lu\n", n);
}

static void
print_tap_result(const char* result_str, unsigned long testnum,
                 const char* msg, va_list ap)
{
    safe_printf("%s %lu", result_str, testnum);

    if (!msg) {
        return;
    }

    safe_printf(" - ");

    safe_vprintf(msg, ap);
}

void
tap_ok_va(unsigned long testnum, const char* msg, va_list ap)
{
    print_tap_result("ok", testnum, msg, ap);
    safe_printf("\n");
}

void
tap_ok_todo_va(unsigned long testnum, const char* msg, va_list ap)
{
    print_tap_result("ok", testnum, msg, ap);
    safe_printf(" # TODO\n");
}

void
tap_ok_skip_va(unsigned long testnum, const char* msg, va_list ap)
{
    print_tap_result("ok", testnum, msg, ap);
    safe_printf(" # SKIP\n");
}

void
tap_not_ok_va(unsigned long testnum, const char* msg, va_list ap)
{
    print_tap_result("not ok", testnum, msg, ap);
    safe_printf("\n");
}

void
tap_not_ok_todo_va(unsigned long testnum, const char* msg, va_list ap)
{
    print_tap_result("not ok", testnum, msg, ap);
    safe_printf(" # TODO \n");
}

void
tap_not_ok_skip_va(unsigned long testnum, const char* msg, va_list ap)
{
    print_tap_result("not ok", testnum, msg, ap);
    safe_printf(" # SKIP \n");
}

void
tap_bail_out_va(const char* desc, va_list ap)
{
    safe_printf("Bail out!");

    if (!desc) {
        goto out;
    }

    safe_printf(" ");
    safe_vprintf(desc, ap);

out:
    safe_printf("\n");
}

void
tap_diag_va(const char* msg, va_list ap)
{
    safe_printf("#");

    if (!msg) {
        goto out;
    }

    safe_printf(" ");
    safe_vprintf(msg, ap);

out:
    safe_printf("\n");
}

void
tap_ok(unsigned long testnum, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_ok_va(testnum, msg, ap);
    va_end(ap);
}

void
tap_ok_todo(unsigned long testnum, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_ok_todo_va(testnum, msg, ap);
    va_end(ap);
}

void
tap_ok_skip(unsigned long testnum, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_ok_skip_va(testnum, msg, ap);
    va_end(ap);
}

void
tap_not_ok(unsigned long testnum, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_not_ok_va(testnum, msg, ap);
    va_end(ap);
}

void
tap_not_ok_todo(unsigned long testnum, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_not_ok_todo_va(testnum, msg, ap);
    va_end(ap);
}

void
tap_not_ok_skip(unsigned long testnum, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_not_ok_skip_va(testnum, msg, ap);
    va_end(ap);
}

void
tap_bail_out(const char* desc, ...)
{
    va_list ap;
    va_start(ap, desc);
    tap_bail_out_va(desc, ap);
    va_end(ap);
}

void
tap_diag(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_diag_va(msg, ap);
    va_end(ap);
}
