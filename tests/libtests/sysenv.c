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

#include "sysenv.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

_Bool
is_cygwin()
{
#if defined(__CYGWIN__)
    return true;
#else
    return false;
#endif
}

_Bool
is_cygwin64()
{
#if defined(__x86_64__)
    return is_cygwin();
#else
    return false;
#endif
}

_Bool
is_freebsd()
{
#if defined(__FreeBSD__)
    return true;
#else
    return false;
#endif
}

_Bool
is_linux()
{
#if defined(__linux__)
    return true;
#else
    return false;
#endif
}

_Bool
is_macos()
{
#if defined(__APPLE__)
    return true;
#else
    return false;
#endif
}

static bool
test_is_valgrind(void)
{
    /* We test for Valgrind by checking for preloaded libraries. */

    const char* str = getenv("LD_PRELOAD");
    if (!str) {
        return false;
    }

    const char* pos = strstr(str, "valgrind/");
    if (!pos) {
        return false;
    }

    return true;
}

_Bool
is_valgrind()
{
    static atomic_bool tested = false;
    static atomic_bool result = false;

    if (atomic_load(&tested)) {
        return atomic_load(&result);
    }

    bool res = test_is_valgrind();

    atomic_store(&result, res);
    atomic_store(&tested, true);

    return res;
}
