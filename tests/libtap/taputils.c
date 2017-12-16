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

#include "taputils.h"
#include <string.h>
#include "tap.h"

/*
 * TAP output with different levels of verbosity
 */

enum tap_verbosity g_tap_verbosity = TAP_ERROR;

void
tap_error_va(const char* msg, va_list ap)
{
    if (g_tap_verbosity < TAP_ERROR) {
        return;
    }
    tap_diag_va(msg, ap);
}

void
tap_info_va(const char* msg, va_list ap)
{
    if (g_tap_verbosity < TAP_INFO) {
        return;
    }
    tap_diag_va(msg, ap);
}

void
tap_debug_va(const char* msg, va_list ap)
{
    if (g_tap_verbosity < TAP_DEBUG) {
        return;
    }
    tap_diag_va(msg, ap);
}

void
tap_error(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_error_va(msg, ap);
    va_end(ap);
}

void
tap_info(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_info_va(msg, ap);
    va_end(ap);
}

void
tap_debug(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    tap_debug_va(msg, ap);
    va_end(ap);
}

/*
 * Error reporting
 */

void
tap_error_errno_at(const char* function, int errnum, const char* filename,
                   unsigned long line)
{
    char buf[100];

    tap_error("%s:%lu: %s failed: %d (%s)", filename, line, function,
              errnum, strerror_r(errnum, buf, sizeof(buf)));
}
