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

#include "tempfile.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "safeblk.h"
#include "safe_pthread.h"
#include "safe_stdlib.h"
#if defined(__MACH__)
#include "safe_unistd.h"
#endif
#include "taputils.h"

static const char  g_template_string[] = "/picotm-XXXXXX";
static char*       g_path = NULL;

static const char*
find_tmpdir(void)
{
    const char* tmpdir = getenv("TMPDIR");
    if (tmpdir) {
        return tmpdir;
    }

    /* fallback to /tmp on Unix */
    tmpdir = "/tmp";

    return tmpdir;
}

static char
get_path_separator(void)
{
    return '/';
}

static const char*
strip_trailing_chr(const char* str, size_t len, char c)
{
    while (len && str[len] == c) {
        --len;
    }
    return str + len;
}

static char*
append_path(char* dst, size_t dstsiz, const char* src, size_t srclen)
{
    if (dstsiz < srclen) {
        tap_error("temporary path exceeds PATH_MAX length");
        abort_safe_block();
    }
    memcpy(dst, src, srclen);
    return dst + srclen;
}

static char*
build_path_template(char* dst, size_t dstsiz, const char* tmpdir)
{
    const char* tmpdirend = strip_trailing_chr(tmpdir, strlen(tmpdir),
                                               get_path_separator());
    size_t tmpdirlen = tmpdirend - tmpdir;

    char* pos = dst;
    char* end = dst + dstsiz;
    pos = append_path(pos, end - pos, tmpdir, tmpdirlen);
    pos = append_path(pos, end - pos, g_template_string,
                      sizeof(g_template_string));

    return pos;
}

static void
atexit_path(void)
{
    if (g_path) {
        free(g_path);
    }
}

const char*
temp_path()
{
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    static char path_template[PATH_MAX]; /* protected by 'lock' */

    safe_pthread_mutex_lock(&lock);

    if (g_path) {
        safe_pthread_mutex_unlock(&lock);
        return g_path;
    }

    const char* tmpdir = find_tmpdir();
    build_path_template(path_template, sizeof(path_template), tmpdir);
    char* path = safe_mkdtemp(path_template);

    g_path = safe_realpath(path, NULL);

    atexit(atexit_path);

    safe_pthread_mutex_unlock(&lock);

    return g_path;
}
