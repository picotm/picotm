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

#include "safe_unistd.h"
#include <errno.h>
#include "compat/temp_failure_retry.h"
#include "safeblk.h"
#include "taputils.h"

int
safe_chdir(const char* path)
{
    int res = TEMP_FAILURE_RETRY(chdir(path));
    if (res < 0) {
        tap_error_errno("chdir()", errno);
        abort_safe_block();
    }
    return res;
}

int
safe_close(int fd)
{
    int res = TEMP_FAILURE_RETRY(close(fd));
    if (res < 0) {
        tap_error_errno("close()", errno);
        abort_safe_block();
    }
    return res;
}

char*
safe_getcwd(char* buf, size_t size)
{
    char* cwd = getcwd(buf, size);;
    if (!cwd) {
        tap_error_errno("getcwd()", errno);
        abort_safe_block();
    }
    return cwd;
}

#if defined(__MACH__)
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
safe_pipe(int pipefd[2])
{
    int res = pipe(pipefd);
    if (res < 0) {
        tap_error_errno("pipe()", errno);
        abort_safe_block();
    }
    return res;
}

ssize_t
safe_pread(int fd, void* buf, size_t count, off_t offset)
{
    size_t len = 0;

    char* buf8 = buf;

    while (len < count) {
        ssize_t res = TEMP_FAILURE_RETRY(pread(fd, buf8, count - len,
                                                   offset + len));
        if (res < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                break; /* non-blocking I/O; return early */
            }
            tap_error_errno("pread()", errno);
            abort_safe_block();
        }
        buf8 += res;
        len += res;
    }

    return len;
}

ssize_t
safe_pwrite(int fd, const void* buf, size_t count, off_t offset)
{
    size_t len = 0;

    const char* buf8 = buf;

    while (len < count) {
        ssize_t res = TEMP_FAILURE_RETRY(pwrite(fd, buf8, count - len,
                                                offset + len));
        if (res < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                break; /* non-blocking I/O; return early */
            }
            tap_error_errno("pwrite()", errno);
            abort_safe_block();
        }
        buf8 += res;
        len += res;
    }

    return len;
}

int
safe_rmdir(const char* path)
{
    int res = rmdir(path);
    if (res < 0) {
        tap_error_errno("rmdir()", errno);
        abort_safe_block();
    }
    return res;
}

int
safe_unlink(const char* pathname)
{
    int res = unlink(pathname);
    if (res < 0) {
        tap_error_errno("unlink()", errno);
        abort_safe_block();
    }
    return res;
}

ssize_t
safe_write(int fd, const void* buf, size_t count)
{
    size_t len = 0;

    const char* buf8 = buf;

    while (len < count) {
        ssize_t res = TEMP_FAILURE_RETRY(write(fd, buf8, count - len));
        if (res < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                break; /* non-blocking I/O; return early */
            }
            tap_error_errno("write()", errno);
            abort_safe_block();
        }
        buf8 += res;
        len += res;
    }

    return len;
}
