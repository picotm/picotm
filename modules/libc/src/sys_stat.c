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
#include <picotm/picotm-tm.h>
#include "error/module.h"
#include "fildes/module.h"
#include "picotm/sys/stat-tm.h"

PICOTM_EXPORT
int
chmod_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return chmod_tm(path, mode);
}

PICOTM_EXPORT
int
fchmod_tx(int fildes, mode_t mode)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_fchmod(fildes, mode);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
fstat_tx(int fildes, struct stat* buf)
{
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return fstat_tm(fildes, buf);
}

PICOTM_EXPORT
int
lstat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return lstat_tm(path, buf);
}

PICOTM_EXPORT
int
mkdir_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mkdir_tm(path, mode);
}

PICOTM_EXPORT
int
mkfifo_tx(const char* path, mode_t mode)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mkfifo_tm(path, mode);
}

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _XOPEN_SOURCE >= 500
PICOTM_EXPORT
int
mknod_tx(const char* path, mode_t mode, dev_t dev)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return mknod_tm(path, mode, dev);
}
#endif

PICOTM_EXPORT
int
stat_tx(const char* path, struct stat* buf)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_tx(buf, sizeof(*buf), PICOTM_TM_PRIVATIZE_STORE);
    return stat_tm(path, buf);
}
