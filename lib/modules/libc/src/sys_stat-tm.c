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

#include "picotm/sys/stat.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "error/module.h"
#include "fildes/module.h"

PICOTM_EXPORT
int
chmod_tm(const char* path, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_chmod(path, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
fstat_tm(int fildes, struct stat* buf)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_fstat(fildes, buf);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
lstat_tm(const char* path, struct stat* buf)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_lstat(path, buf);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
mkdir_tm(const char* path, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_mkdir(path, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
mkfifo_tm(const char* path, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_mkfifo(path, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
PICOTM_EXPORT
int
mknod_tm(const char* path, mode_t mode, dev_t dev)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_mknod(path, mode, dev);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

PICOTM_EXPORT
int
stat_tm(const char* path, struct stat* buf)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_stat(path, buf);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
