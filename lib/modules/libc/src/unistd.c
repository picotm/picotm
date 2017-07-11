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

#include "picotm/unistd.h"
#include <errno.h>
#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include "error/module.h"
#include "fd/module.h"
#include "picotm/unistd-tm.h"

PICOTM_EXPORT
void
_exit_tx(int status)
{
    __picotm_commit();
    _exit(status);
}

PICOTM_EXPORT
int
chdir_tx(const char* path)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return chdir_tm(path);
}

PICOTM_EXPORT
int
close_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_close(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
dup_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_dup(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

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

PICOTM_EXPORT
int
fchdir_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_fchdir(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
fsync_tx(int fildes)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_fsync(fildes);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
char*
getcwd_tx(char* buf, size_t size)
{
    privatize_tx(buf, size, PICOTM_TM_PRIVATIZE_LOAD);
    return getcwd_tm(buf, size);
}

PICOTM_EXPORT
int
link_tx(const char* path1, const char* path2)
{
    privatize_c_tx(path1, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    privatize_c_tx(path2, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return link_tm(path1, path2);
}

PICOTM_EXPORT
off_t
lseek_tx(int fildes, off_t offset, int whence)
{
    error_module_save_errno();

    off_t res;

    do {
        res = fd_module_lseek(fildes, offset, whence);
        if (res == (off_t)-1) {
            picotm_recover_from_errno(errno);
        }
    } while (res == (off_t)-1);

    return res;
}

PICOTM_EXPORT
int
pipe_tx(int fildes[2])
{
    privatize_tx(fildes, 2 * sizeof(fildes[0]), PICOTM_TM_PRIVATIZE_STORE);
    return pipe_tm(fildes);
}

PICOTM_EXPORT
ssize_t
pread_tx(int fildes, void* buf, size_t nbyte, off_t offset)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_STORE);
    return pread_tm(fildes, buf, nbyte, offset);
}

PICOTM_EXPORT
ssize_t
pwrite_tx(int fildes, const void* buf, size_t nbyte, off_t offset)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_LOAD);
    return pwrite_tm(fildes, buf, nbyte, offset);
}

PICOTM_EXPORT
ssize_t
read_tx(int fildes, void* buf, size_t nbyte)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_STORE);
    return read_tm(fildes, buf, nbyte);
}

PICOTM_EXPORT
unsigned
sleep_tx(unsigned seconds)
{
    do {
        seconds = sleep(seconds);
    } while (seconds);
    return 0;
}

PICOTM_EXPORT
void
sync_tx()
{
    fd_module_sync();
}

PICOTM_EXPORT
int
unlink_tx(const char* path)
{
    privatize_c_tx(path, '\0', PICOTM_TM_PRIVATIZE_LOAD);
    return unlink_tm(path);
}

PICOTM_EXPORT
ssize_t
write_tx(int fildes, const void* buf, size_t nbyte)
{
    privatize_tx(buf, nbyte, PICOTM_TM_PRIVATIZE_LOAD);
    return write_tm(fildes, buf, nbyte);
}
