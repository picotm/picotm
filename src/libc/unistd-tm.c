/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/unistd-tm.h"
#include <systx/systx.h>
#include "fd/comfdtx.h"
#include "fs/comfstx.h"

SYSTX_EXPORT
int
chdir_tm(const char* path)
{
    return com_fs_tx_chdir(path);
}

SYSTX_EXPORT
char*
getcwd_tm(char* buf, size_t size)
{
    return com_fs_tx_getcwd(buf, size);
}

SYSTX_EXPORT
int
link_tm(const char* path1, const char* path2)
{
    return com_fs_tx_link(path1, path2);
}

SYSTX_EXPORT
int
pipe_tm(int fildes[2])
{
    return com_fd_tx_pipe(fildes);
}

SYSTX_EXPORT
ssize_t
pread_tm(int fildes, void* buf, size_t nbyte, off_t offset)
{
    return com_fd_tx_pread(fildes, buf, nbyte, offset);
}

SYSTX_EXPORT
ssize_t
pwrite_tm(int fildes, const void* buf, size_t nbyte, off_t offset)
{
    return com_fd_tx_pwrite(fildes, buf, nbyte, offset);
}

SYSTX_EXPORT
ssize_t
read_tm(int fildes, void* buf, size_t nbyte)
{
    return com_fd_tx_read(fildes, buf, nbyte);
}

SYSTX_EXPORT
int
unlink_tm(const char* path)
{
    return com_fs_tx_unlink(path);
}

SYSTX_EXPORT
ssize_t
write_tm(int fildes, const void* buf, size_t nbyte)
{
    return com_fd_tx_write(fildes, buf, nbyte);
}
