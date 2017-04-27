/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/unistd-tm.h"
#include <errno.h>
#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include "error/module.h"
#include "fd/module.h"
#include "fs/module.h"

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

PICOTM_EXPORT
int
chdir_tm(const char* path)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_chdir(path);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
char*
getcwd_tm(char* buf, size_t size)
{
    error_module_save_errno();

    char* str;

    do {
        str = vfs_module_getcwd(buf, size);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);

    return str;
}

PICOTM_EXPORT
int
link_tm(const char* path1, const char* path2)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_link(path1, path2);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
pipe_tm(int fildes[2])
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_pipe(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
pread_tm(int fildes, void* buf, size_t nbyte, off_t offset)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fd_module_pread(fildes, buf, nbyte, offset);
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

PICOTM_EXPORT
ssize_t
pwrite_tm(int fildes, const void* buf, size_t nbyte, off_t offset)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fd_module_pwrite(fildes, buf, nbyte, offset);
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

PICOTM_EXPORT
ssize_t
read_tm(int fildes, void* buf, size_t nbyte)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fd_module_read(fildes, buf, nbyte);
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

PICOTM_EXPORT
int
unlink_tm(const char* path)
{
    error_module_save_errno();

    int res;

    do {
        res = vfs_module_unlink(path);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
write_tm(int fildes, const void* buf, size_t nbyte)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_write(fildes, buf, nbyte);
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
