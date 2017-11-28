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
 * A transaction-safe implementation of [chmod()][posix::chmod].
 *
 * [posix::chmod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/chmod.html
 */
int
chmod_tx(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fchmod()][posix::fchmod].
 *
 * [posix::fchmod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fchmod.html
 */
int
fchmod_tx(int fildes, mode_t mode);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fstat()][posix::fstat].
 *
 * [posix::fstat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fstat.html
 */
int
fstat_tx(int fildes, struct stat* buf);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lstat()][posix::lstat].
 *
 * [posix::lstat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lstat.html
 */
int
lstat_tx(const char* path, struct stat* buf);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [mkdir()][posix::mkdir].
 *
 * [posix::mkdir]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkdir.html
 */
int
mkdir_tx(const char* path, mode_t mode);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [mkfifo()][posix::mkfifo].
 *
 * [posix::mkfifo]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mkfifo.html
 */
int
mkfifo_tx(const char* path, mode_t mode);

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
/**
 * A transaction-safe implementation of [mknod()][posix::mknod].
 *
 * [posix::mknod]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/mknod.html
 */
PICOTM_NOTHROW
int
mknod_tx(const char* path, mode_t mode, dev_t dev);
#endif

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [stat()][posix::stat].
 *
 * [posix::stat]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/stat.html
 */
int
stat_tx(const char* path, struct stat* buf);

PICOTM_END_DECLS
