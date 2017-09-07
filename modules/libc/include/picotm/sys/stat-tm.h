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
#include <sys/stat.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sys/stat.h>.
 */

PICOTM_NOTHROW
/**
 * Variant of chmod_tx() that operates on transactional memory.
 */
int
chmod_tm(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * Variant of fstat_tx() that operates on transactional memory.
 */
int
fstat_tm(int fildes, struct stat* buf);

PICOTM_NOTHROW
/**
 * Variant of lstat_tx() that operates on transactional memory.
 */
int
lstat_tm(const char* path, struct stat* buf);

PICOTM_NOTHROW
/**
 * Variant of mkdir_tx() that operates on transactional memory.
 */
int
mkdir_tm(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * Variant of mkfifo_tx() that operates on transactional memory.
 */
int
mkfifo_tm(const char* path, mode_t mode);

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
/**
 * Variant of mknod_tx() that operates on transactional memory.
 */
PICOTM_NOTHROW
int
mknod_tm(const char* path, mode_t mode, dev_t dev);
#endif

PICOTM_NOTHROW
/**
 * Variant of stat_tx() that operates on transactional memory.
 */
int
stat_tm(const char* path, struct stat* buf);

PICOTM_END_DECLS
