/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "picotm/unistd.h"
#include "picotm/picotm.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-tm.h"
#include "picotm/unistd-tm.h"
#include <errno.h>
#include "error/module.h"
#include "fildes/module.h"

#if defined(PICOTM_LIBC_HAVE__EXIT) && PICOTM_LIBC_HAVE__EXIT
PICOTM_EXPORT
void
_exit_tx(int status)
{
    __picotm_commit();
    _exit(status);
}
#endif

#if defined(PICOTM_LIBC_HAVE_CHDIR) && PICOTM_LIBC_HAVE_CHDIR
PICOTM_EXPORT
int
chdir_tx(const char* path)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return chdir_tm(path);
}
#endif

#if defined(PICOTM_LIBC_HAVE_CLOSE) && PICOTM_LIBC_HAVE_CLOSE
PICOTM_EXPORT
int
close_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_close(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_DUP) && PICOTM_LIBC_HAVE_DUP
PICOTM_EXPORT
int
dup_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_dup(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_DUP2) && PICOTM_LIBC_HAVE_DUP2
PICOTM_EXPORT
int
dup2_tx(int fildes, int fildes2)
{
    picotm_irrevocable();
    error_module_save_errno();

    int res;

    do {
        res = dup2(fildes, fildes2);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_FCHDIR) && PICOTM_LIBC_HAVE_FCHDIR
PICOTM_EXPORT
int
fchdir_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_fchdir(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_FSYNC) && PICOTM_LIBC_HAVE_FSYNC
PICOTM_EXPORT
int
fsync_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_fsync(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_GETCWD) && PICOTM_LIBC_HAVE_GETCWD
PICOTM_EXPORT
char*
getcwd_tx(char* buf, size_t size)
{
    privatize_tx(buf, size, PICOTM_TM_PRIVATIZE_LOAD);
    return getcwd_tm(buf, size);
}
#endif

#if defined(PICOTM_LIBC_HAVE_LINK) && PICOTM_LIBC_HAVE_LINK
PICOTM_EXPORT
int
link_tx(const char* path1, const char* path2)
{
    privatize_c_tx(path1, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(path2, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return link_tm(path1, path2);
}
#endif

#if defined(PICOTM_LIBC_HAVE_LSEEK) && PICOTM_LIBC_HAVE_LSEEK
PICOTM_EXPORT
off_t
lseek_tx(int fildes, off_t offset, int whence)
{
    error_module_save_errno();

    off_t res;

    do {
        res = fildes_module_lseek(fildes, offset, whence);
        if (res == (off_t)-1) {
            picotm_recover_from_errno(errno);
        }
    } while (res == (off_t)-1);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_MKDTEMP) && PICOTM_LIBC_HAVE_MKDTEMP && \
    defined(__MACH__)
PICOTM_EXPORT
char*
mkdtemp_tx(char* template)
{
    privatize_c_tx(template, '\0', PICOTM_TM_PRIVATIZE_LOADSTORE);
    return mkdtemp_tm(template);
}
#endif

#if defined(PICOTM_LIBC_HAVE_PIPE) && PICOTM_LIBC_HAVE_PIPE
PICOTM_EXPORT
int
pipe_tx(int fildes[2])
{
    privatize_tx(fildes, 2 * sizeof(fildes[0]), PICOTM_TM_PRIVATIZE_STORE);
    return pipe_tm(fildes);
}
#endif

#if defined(PICOTM_LIBC_HAVE_PREAD) && PICOTM_LIBC_HAVE_PREAD
PICOTM_EXPORT
ssize_t
pread_tx(int fildes, void* buf, size_t nbyte, off_t offset)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_STORE);
    return pread_tm(fildes, buf, nbyte, offset);
}
#endif

#if defined(PICOTM_LIBC_HAVE_PWRITE) && PICOTM_LIBC_HAVE_PWRITE
PICOTM_EXPORT
ssize_t
pwrite_tx(int fildes, const void* buf, size_t nbyte, off_t offset)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_LOAD);
    return pwrite_tm(fildes, buf, nbyte, offset);
}
#endif

#if defined(PICOTM_LIBC_HAVE_READ) && PICOTM_LIBC_HAVE_READ
PICOTM_EXPORT
ssize_t
read_tx(int fildes, void* buf, size_t nbyte)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_STORE);
    return read_tm(fildes, buf, nbyte);
}
#endif

#if defined(PICOTM_LIBC_HAVE_SLEEP) && PICOTM_LIBC_HAVE_SLEEP
PICOTM_EXPORT
unsigned
sleep_tx(unsigned seconds)
{
    do {
        seconds = sleep(seconds);
    } while (seconds);
    return 0;
}
#endif

#if defined(PICOTM_LIBC_HAVE_SYNC) && PICOTM_LIBC_HAVE_SYNC
PICOTM_EXPORT
void
sync_tx()
{
    fildes_module_sync();
}
#endif

#if defined(PICOTM_LIBC_HAVE_UNLINK) && PICOTM_LIBC_HAVE_UNLINK
PICOTM_EXPORT
int
unlink_tx(const char* path)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return unlink_tm(path);
}
#endif

#if defined(PICOTM_LIBC_HAVE_WRITE) && PICOTM_LIBC_HAVE_WRITE
PICOTM_EXPORT
ssize_t
write_tx(int fildes, const void* buf, size_t nbyte)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_LOAD);
    return write_tm(fildes, buf, nbyte);
}
#endif
