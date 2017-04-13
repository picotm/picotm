/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/unistd.h"
#include <systx/systx.h>
#include <systx/systx-module.h>
#include <systx/systx-tm.h>
#include "fd/comfdtx.h"
#include "fs/comfstx.h"
#include "systx/unistd-tm.h"

SYSTX_EXPORT
void
_exit_tx(int status)
{
    __systx_commit();
    _exit(status);
}

SYSTX_EXPORT
int
chdir_tx(const char* path)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return chdir_tm(path);
}

SYSTX_EXPORT
int
close_tx(int fildes)
{
    return com_fd_tx_close(fildes);
}

SYSTX_EXPORT
int
dup_tx(int fildes)
{
    return com_fd_tx_dup(fildes);
}

SYSTX_EXPORT
int
dup2_tx(int fildes, int fildes2)
{
    systx_irrevocable();
    return dup2(fildes, fildes2);
}

SYSTX_EXPORT
int
fchdir_tx(int fildes)
{
    return com_fs_tx_fchdir(fildes);
}

SYSTX_EXPORT
int
fsync_tx(int fildes)
{
    return com_fd_tx_fsync(fildes);
}

SYSTX_EXPORT
char*
getcwd_tx(char* buf, size_t size)
{
    privatize_tx(buf, size, SYSTX_TM_PRIVATIZE_LOAD);
    return getcwd_tm(buf, size);
}

SYSTX_EXPORT
int
link_tx(const char* path1, const char* path2)
{
    privatize_c_tx(path1, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    privatize_c_tx(path2, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return link_tm(path1, path2);
}

SYSTX_EXPORT
off_t
lseek_tx(int fildes, off_t offset, int whence)
{
    return com_fd_tx_lseek(fildes, offset, whence);
}

SYSTX_EXPORT
int
pipe_tx(int fildes[2])
{
    privatize_tx(fildes, 2 * sizeof(fildes[0]), SYSTX_TM_PRIVATIZE_STORE);
    return pipe_tm(fildes);
}

SYSTX_EXPORT
ssize_t
pread_tx(int fildes, void* buf, size_t nbyte, off_t offset)
{
    privatize_tx(buf, nbyte, SYSTX_TM_PRIVATIZE_STORE);
    return pread_tm(fildes, buf, nbyte, offset);
}

SYSTX_EXPORT
ssize_t
pwrite_tx(int fildes, const void* buf, size_t nbyte, off_t offset)
{
    privatize_tx(buf, nbyte, SYSTX_TM_PRIVATIZE_LOAD);
    return pwrite_tm(fildes, buf, nbyte, offset);
}

SYSTX_EXPORT
ssize_t
read_tx(int fildes, void* buf, size_t nbyte)
{
    privatize_tx(buf, nbyte, SYSTX_TM_PRIVATIZE_STORE);
    return read_tm(fildes, buf, nbyte);
}

SYSTX_EXPORT
void
sync_tx()
{
    com_fd_tx_sync();
}

SYSTX_EXPORT
int
unlink_tx(const char* path)
{
    privatize_c_tx(path, '\0', SYSTX_TM_PRIVATIZE_LOAD);
    return unlink_tm(path);
}

SYSTX_EXPORT
ssize_t
write_tx(int fildes, const void* buf, size_t nbyte)
{
    privatize_tx(buf, nbyte, SYSTX_TM_PRIVATIZE_LOAD);
    return write_tm(fildes, buf, nbyte);
}
