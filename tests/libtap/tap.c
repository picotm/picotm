/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
