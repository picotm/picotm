/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/fcntl-tm.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "fd/comfdtx.h"

SYSTX_EXPORT
int
creat_tm(const char* path, mode_t mode)
{
    return open_tm(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

SYSTX_EXPORT
int
fcntl_tm(int fildes, int cmd, ...)
{
    switch (cmd) {
        case F_DUPFD:
            /* Handle like dup() */
            return com_fd_tx_dup_internal(fildes, false);
        case F_DUPFD_CLOEXEC:
            /* Handle like dup() with CLOEXEC */
            return com_fd_tx_dup_internal(fildes, true);
        case F_SETFD:
        case F_SETFL:
        case F_SETOWN:
            {
                union com_fd_fcntl_arg val;
                va_list arg;
                va_start(arg, cmd);
                val.arg0 = va_arg(arg, int);
                va_end(arg);

                return com_fd_tx_fcntl(fildes, cmd, &val);
            }
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW:
            {
                union com_fd_fcntl_arg val;
                struct flock* f;
                va_list arg;
                va_start(arg, cmd);
                f = va_arg(arg, struct flock*);
                va_end(arg);

                memcpy(&val.arg1, f, sizeof(val.arg1));

                return com_fd_tx_fcntl(fildes, cmd, &val);
            }
        default:
            return com_fd_tx_fcntl(fildes, cmd, NULL);
    }
}

SYSTX_EXPORT
int
open_tm(const char* path, int oflag, ...)
{
    mode_t mode = 0;

    if (oflag & O_CREAT) {
        va_list arg;
        va_start(arg, oflag);
        mode = va_arg(arg, mode_t);
        va_end(arg);
    }

    int res = com_fd_tx_open(path, oflag, mode);
    if (res < 0) {
        return -1;
    }
    return res;
}
