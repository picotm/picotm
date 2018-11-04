/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "safe_stdlib.h"
#include <errno.h>
#include <stdbool.h>
#include "safeblk.h"
#include "taputils.h"

void*
safe_calloc(size_t nmemb, size_t size)
{
    void* mem = calloc(nmemb, size);
    if (size && !mem) {
        tap_error_errno("calloc()", size);
        abort_safe_block();
    }
    return mem;
}

void*
safe_malloc(size_t size)
{
    void* mem = malloc(size);
    if (size && !mem) {
        tap_error_errno("malloc()", errno);
        abort_safe_block();
    }
    return mem;
}

#if !defined(__MACH__)
char*
safe_mkdtemp(char* tmplate)
{
    char* path = mkdtemp(tmplate);
    if (!path) {
        tap_error_errno("mkdtemp()", errno);
        abort_safe_block();
    }
    return path;
}
#endif

int
safe_mkstemp(char* tmplate)
{
    int res = mkstemp(tmplate);
    if (res < 0) {
        tap_error_errno("mkstemp()", errno);
        abort_safe_block();
    }
    return res;
}

static bool
string_is_empty(const char* str)
{
    return !(*str);
}

char*
safe_mktemp(char* tmplate)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    char* filename = mktemp(tmplate);
#pragma GCC diagnostic pop
    if (string_is_empty(filename)) {
        tap_error_errno("mktemp()", errno);
        abort_safe_block();
    }
    return filename;
}

char*
safe_realpath(const char* path, char* resolved_path)
{
    char* res = realpath(path, resolved_path);
    if (!res) {
        tap_error_errno("realpath()", errno);
        abort_safe_block();
    }
    return res;
}
