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
#include <picotm/config/picotm-libc-config.h>
#include <unistd.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <unistd.h>.
 */

#if defined(PICOTM_LIBC_HAVE_CHDIR) && PICOTM_LIBC_HAVE_CHDIR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of chdir_tx() that operates on transactional memory.
 */
int
chdir_tm(const char* path);
#endif

#if defined(PICOTM_LIBC_HAVE_GETCWD) && PICOTM_LIBC_HAVE_GETCWD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of getcwd_tx() that operates on transactional memory.
 */
char*
getcwd_tm(char* buf, size_t size);
#endif

#if defined(PICOTM_LIBC_HAVE_LINK) && PICOTM_LIBC_HAVE_LINK || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of link_tx() that operates on transactional memory.
 */
int
link_tm(const char* path1, const char* path2);
#endif

#if defined(PICOTM_LIBC_HAVE_PIPE) && PICOTM_LIBC_HAVE_PIPE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of pipe_tx() that operates on transactional memory.
 */
int
pipe_tm(int fildes[2]);
#endif

#if defined(PICOTM_LIBC_HAVE_PREAD) && PICOTM_LIBC_HAVE_PREAD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of pread_tx() that operates on transactional memory.
 */
ssize_t
pread_tm(int fildes, void* buf, size_t nbyte, off_t offset);
#endif

#if defined(PICOTM_LIBC_HAVE_PWRITE) && PICOTM_LIBC_HAVE_PWRITE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of pwrite_tx() that operates on transactional memory.
 */
ssize_t
pwrite_tm(int fildes, const void* buf, size_t nbyte, off_t offset);
#endif

#if defined(PICOTM_LIBC_HAVE_READ) && PICOTM_LIBC_HAVE_READ || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of read_tx() that operates on transactional memory.
 */
ssize_t
read_tm(int fildes, void* buf, size_t nbyte);
#endif

#if defined(PICOTM_LIBC_HAVE_UNLINK) && PICOTM_LIBC_HAVE_UNLINK || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of unlink_tx() that operates on transactional memory.
 */
int
unlink_tm(const char* path);
#endif

#if defined(PICOTM_LIBC_HAVE_WRITE) && PICOTM_LIBC_HAVE_WRITE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * Variant of write_tx() that operates on transactional memory.
 */
ssize_t
write_tm(int fildes, const void* buf, size_t nbyte);
#endif

PICOTM_END_DECLS
