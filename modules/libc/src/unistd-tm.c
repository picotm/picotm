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

#include "picotm/unistd-tm.h"
#include <errno.h>
#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include "cwd/module.h"
#include "error/module.h"
#include "fildes/module.h"

static bool
perform_recovery(int errno_hint)
{
    enum picotm_libc_error_recovery recovery =
        picotm_libc_get_error_recovery();

    if (recovery == PICOTM_LIBC_ERROR_RECOVERY_FULL) {
        return true;
    }

    return (errno_hint != EAGAIN) && (errno_hint != EWOULDBLOCK);
}

#if defined(PICOTM_LIBC_HAVE_CHDIR) && PICOTM_LIBC_HAVE_CHDIR
PICOTM_EXPORT
int
chdir_tm(const char* path)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = cwd_module_chdir(path, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_GETCWD) && PICOTM_LIBC_HAVE_GETCWD
PICOTM_EXPORT
char*
getcwd_tm(char* buf, size_t size)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        char* cwd = cwd_module_getcwd(buf, size, &error);
        if (!picotm_error_is_set(&error)) {
            return cwd;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_LINK) && PICOTM_LIBC_HAVE_LINK
PICOTM_EXPORT
int
link_tm(const char* path1, const char* path2)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_link(path1, path2);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_PIPE) && PICOTM_LIBC_HAVE_PIPE
PICOTM_EXPORT
int
pipe_tm(int fildes[2])
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_pipe(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_PREAD) && PICOTM_LIBC_HAVE_PREAD
PICOTM_EXPORT
ssize_t
pread_tm(int fildes, void* buf, size_t nbyte, off_t offset)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_pread(fildes, buf, nbyte, offset);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_PWRITE) && PICOTM_LIBC_HAVE_PWRITE
PICOTM_EXPORT
ssize_t
pwrite_tm(int fildes, const void* buf, size_t nbyte, off_t offset)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_pwrite(fildes, buf, nbyte, offset);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_READ) && PICOTM_LIBC_HAVE_READ
PICOTM_EXPORT
ssize_t
read_tm(int fildes, void* buf, size_t nbyte)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_read(fildes, buf, nbyte);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_UNLINK) && PICOTM_LIBC_HAVE_UNLINK
PICOTM_EXPORT
int
unlink_tm(const char* path)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_unlink(path);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_WRITE) && PICOTM_LIBC_HAVE_WRITE
PICOTM_EXPORT
ssize_t
write_tm(int fildes, const void* buf, size_t nbyte)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_write(fildes, buf, nbyte);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif
