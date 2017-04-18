/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/unistd.h"
#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include "fd/comfdtx.h"
#include "fs/comfstx.h"
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
    return com_fd_tx_close(fildes);
}

PICOTM_EXPORT
int
dup_tx(int fildes)
{
    return com_fd_tx_dup(fildes);
}

PICOTM_EXPORT
int
dup2_tx(int fildes, int fildes2)
{
    picotm_irrevocable();
    return dup2(fildes, fildes2);
}

PICOTM_EXPORT
int
fchdir_tx(int fildes)
{
    return com_fs_tx_fchdir(fildes);
}

PICOTM_EXPORT
int
fsync_tx(int fildes)
{
    return com_fd_tx_fsync(fildes);
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
    return com_fd_tx_lseek(fildes, offset, whence);
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
void
sync_tx()
{
    com_fd_tx_sync();
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
