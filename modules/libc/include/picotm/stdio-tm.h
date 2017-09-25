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

#include <picotm/compiler.h>
#include <stdarg.h>
#include <stdio.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <stdio.h>.
 */

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [snprintf()][posix::snprintf].
 *
 * [posix::snprintf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/snprintf.html
 */
int
snprintf_tm(char* restrict s, size_t n, const char* restrict format, ...);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sscanf()][posix::sscanf].
 *
 * [posix::sscanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sscanf.html
 */
int
sscanf_tm(const char* restrict s, const char* restrict format, ...);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [vsnprintf()][posix::vsnprintf].
 *
 * [posix::vsnprintf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/vsnprintf.html
 */
int
vsnprintf_tm(char* restrict s, size_t n, const char* restrict format,
             va_list ap);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [vsscanf()][posix::vsscanf].
 *
 * [posix::vsscanf]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/vsscanf.html
 */
int
vsscanf_tm(const char* restrict s, const char* restrict format, va_list arg);

PICOTM_END_DECLS