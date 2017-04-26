/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/fcntl.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "error/module.h"
#include "fd/module.h"
#include "picotm/fcntl-tm.h"

PICOTM_EXPORT
int
creat_tx(const char* path, mode_t mode)
{
    return open_tx(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

PICOTM_EXPORT
int
fcntl_tx(int fildes, int cmd, ...)
{
    error_module_save_errno();

    int res;

    do {
        switch (cmd) {
            case F_DUPFD:
                /* Handle like dup() */
                res = fd_module_dup_internal(fildes, false);
            case F_DUPFD_CLOEXEC:
                /* Handle like dup() with CLOEXEC */
                res = fd_module_dup_internal(fildes, true);
            case F_SETFD:
            case F_SETFL:
            case F_SETOWN:
            {
                union fcntl_arg val;
                va_list arg;
                va_start(arg, cmd);
                val.arg0 = va_arg(arg, int);
                va_end(arg);

                res = fd_module_fcntl(fildes, cmd, &val);
            }
            case F_GETLK:
            case F_SETLK:
            case F_SETLKW:
            {
                union fcntl_arg val;
                struct flock* f;
                va_list arg;
                va_start(arg, cmd);
                f = va_arg(arg, struct flock*);
                va_end(arg);

                privatize_tx(f, sizeof(val.arg1), PICOTM_TM_PRIVATIZE_LOAD);
                memcpy(&val.arg1, f, sizeof(val.arg1));

                res = fd_module_fcntl(fildes, cmd, &val);
            }
            default:
                res = fd_module_fcntl(fildes, cmd, NULL);
        }
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
open_tx(const char* path, int oflag, ...)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);

    if (!(oflag & O_CREAT)) {
        return open_tm(path, oflag);
    }

    va_list arg;
    va_start(arg, oflag);
    mode_t mode = va_arg(arg, mode_t);
    va_end(arg);

    return open_tm(path, oflag, mode);
}
