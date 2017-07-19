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

#include "picotm/fcntl-tm.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "error/module.h"
#include "fd/module.h"

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
                res = fd_module_dup_internal(fildes, false);
                break;
            case F_DUPFD_CLOEXEC:
                /* Handle like dup() with CLOEXEC */
                res = fd_module_dup_internal(fildes, true);
                break;
            case F_SETFD:
            case F_SETFL:
            case F_SETOWN: {
                union fcntl_arg val;
                va_list arg;
                va_start(arg, cmd);
                val.arg0 = va_arg(arg, int);
                va_end(arg);

                res = fd_module_fcntl(fildes, cmd, &val);
                break;
            }
            case F_GETLK:
            case F_SETLK:
            case F_SETLKW: {
                union fcntl_arg val;
                struct flock* f;
                va_list arg;
                va_start(arg, cmd);
                f = va_arg(arg, struct flock*);
                va_end(arg);

                memcpy(&val.arg1, f, sizeof(val.arg1));

                res = fd_module_fcntl(fildes, cmd, &val);
                break;
            }
            default:
                res = fd_module_fcntl(fildes, cmd, NULL);
                break;
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
        res = fd_module_open(path, oflag, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
