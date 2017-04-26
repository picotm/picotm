/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/fcntl-tm.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "error/module.h"
#include "fd/comfdtx.h"

PICOTM_EXPORT
int
creat_tm(const char* path, mode_t mode)
{
    return open_tm(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

PICOTM_EXPORT
int
fcntl_tm(int fildes, int cmd, ...)
{
    error_module_save_errno();

    int res;

    do {
        switch (cmd) {
            case F_DUPFD:
                /* Handle like dup() */
                res = com_fd_tx_dup_internal(fildes, false);
            case F_DUPFD_CLOEXEC:
                /* Handle like dup() with CLOEXEC */
                res = com_fd_tx_dup_internal(fildes, true);
            case F_SETFD:
            case F_SETFL:
            case F_SETOWN:
            {
                union com_fd_fcntl_arg val;
                va_list arg;
                va_start(arg, cmd);
                val.arg0 = va_arg(arg, int);
                va_end(arg);

                res = com_fd_tx_fcntl(fildes, cmd, &val);
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

                res = com_fd_tx_fcntl(fildes, cmd, &val);
            }
            default:
                res = com_fd_tx_fcntl(fildes, cmd, NULL);
        }
        if (res < 0) {
            picotm_recover_from_errno(errno);
        } else {

        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
open_tm(const char* path, int oflag, ...)
{
    error_module_save_errno();

    mode_t mode = 0;

    if (oflag & O_CREAT) {
        va_list arg;
        va_start(arg, oflag);
        mode = va_arg(arg, mode_t);
        va_end(arg);
    }

    int res;

    do {
        res = com_fd_tx_open(path, oflag, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
