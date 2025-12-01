/*
 * picotm - A system-level transaction manager
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
