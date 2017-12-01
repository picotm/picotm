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

#if defined(PICOTM_LIBC_HAVE__EXIT) && PICOTM_LIBC_HAVE__EXIT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW PICOTM_NORETURN
/**
 * A transaction-safe implementation of [_exit()][posix::_exit].
 *
 * \bug Calling _exit() simply terminates the process. It probably should not
 *      be available in a transaction.
 *
 * [posix::_exit]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/_exit.html
 */
void
_exit_tx(int status);
#endif

#if defined(PICOTM_LIBC_HAVE_CHDIR) && PICOTM_LIBC_HAVE_CHDIR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [chdir()][posix::chdir].
 *
 * [posix::chdir]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/chdir.html
 */
int
chdir_tx(const char* path);
#endif

#if defined(PICOTM_LIBC_HAVE_CLOSE) && PICOTM_LIBC_HAVE_CLOSE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [close()][posix::close].
 *
 * [posix::close]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/close.html
 */
int
close_tx(int fildes);
#endif

#if defined(PICOTM_LIBC_HAVE_DUP) && PICOTM_LIBC_HAVE_DUP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [dup()][posix::dup].
 *
 * [posix::dup]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/dup.html
 */
int
dup_tx(int fildes);
#endif

#if defined(PICOTM_LIBC_HAVE_DUP2) && PICOTM_LIBC_HAVE_DUP2 || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [dup2()][posix::dup2].
 *
 * [posix::dup2]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/dup2.html
 */
int
dup2_tx(int fildes, int fildes2);
#endif

#if defined(PICOTM_LIBC_HAVE_FCHDIR) && PICOTM_LIBC_HAVE_FCHDIR || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fchdir()][posix::fchdir].
 *
 * [posix::fchdir]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fchdir.html
 */
int
fchdir_tx(int fildes);
#endif

#if defined(PICOTM_LIBC_HAVE_FSYNC) && PICOTM_LIBC_HAVE_FSYNC || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [fsync()][posix::fsync].
 *
 * [posix::fsync]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/fsync.html
 */
int
fsync_tx(int fildes);
#endif

#if defined(PICOTM_LIBC_HAVE_GETCWD) && PICOTM_LIBC_HAVE_GETCWD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [getcwd()][posix::getcwd].
 *
 * [posix::getcwd]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/getcwd.html
 */
char*
getcwd_tx(char* buf, size_t size);
#endif

#if defined(PICOTM_LIBC_HAVE_LINK) && PICOTM_LIBC_HAVE_LINK || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [link()][posix::link].
 *
 * [posix::link]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/link.html
 */
int
link_tx(const char* path1, const char* path2);
#endif

#if defined(PICOTM_LIBC_HAVE_LSEEK) && PICOTM_LIBC_HAVE_LSEEK || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [lseek()][posix::lseek].
 *
 * [posix::lseek]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/lseek.html
 */
off_t
lseek_tx(int fildes, off_t offset, int whence);
#endif

#if defined(PICOTM_LIBC_HAVE_PIPE) && PICOTM_LIBC_HAVE_PIPE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pipe()][posix::pipe].
 *
 * [posix::pipe]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pipe.html
 */
int
pipe_tx(int fildes[2]);
#endif

#if defined(PICOTM_LIBC_HAVE_PREAD) && PICOTM_LIBC_HAVE_PREAD || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pread()][posix::pread].
 *
 * [posix::pread]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pread.html
 */
ssize_t
pread_tx(int fildes, void* buf, size_t nbyte, off_t offset);
#endif

#if defined(PICOTM_LIBC_HAVE_PWRITE) && PICOTM_LIBC_HAVE_PWRITE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [pwrite()][posix::pwrite].
 *
 * [posix::pwrite]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/pwrite.html
 */
ssize_t
pwrite_tx(int fildes, const void* buf, size_t nbyte, off_t offset);
#endif

#if defined(PICOTM_LIBC_HAVE_READ) && PICOTM_LIBC_HAVE_READ || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [read()][posix::read].
 *
 * [posix::read]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html
 */
ssize_t
read_tx(int fildes, void* buf, size_t nbyte);
#endif

#if defined(PICOTM_LIBC_HAVE_SLEEP) && PICOTM_LIBC_HAVE_SLEEP || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sleep()][posix::sleep].
 *
 * [posix::sleep]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sleep.html
 */
unsigned
sleep_tx(unsigned seconds);
#endif

#if defined(PICOTM_LIBC_HAVE_SYNC) && PICOTM_LIBC_HAVE_SYNC || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sync()][posix::sync].
 *
 * [posix::sync]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sync.html
 */
void
sync_tx(void);
#endif

#if defined(PICOTM_LIBC_HAVE_UNLINK) && PICOTM_LIBC_HAVE_UNLINK || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [unlink()][posix::unlink].
 *
 * [posix::unlink]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/unlink.html
 */
int
unlink_tx(const char* path);
#endif

#if defined(PICOTM_LIBC_HAVE_WRITE) && PICOTM_LIBC_HAVE_WRITE || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [write()][posix::write].
 *
 * [posix::write]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/write.html
 */
ssize_t
write_tx(int fildes, const void* buf, size_t nbyte);
#endif

PICOTM_END_DECLS
