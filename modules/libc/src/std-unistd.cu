/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_UNISTD_H);
ceuta_hdrl(#define TANGER_STM_UNISTD_H);
ceuta_hdrl(#include <unistd.h>);

#include <unistd.h>

ceuta_excl(void, fork, fork);

extern int     com_fd_tx_dup(int);
extern int     com_fd_tx_close(int);
extern ssize_t com_fd_tx_pwrite(int, const void*, size_t, off_t);
extern ssize_t com_fd_tx_pread(int, void*, size_t, off_t);
extern ssize_t com_fd_tx_write(int, const void*, size_t);
extern ssize_t com_fd_tx_read(int, void*, size_t);
extern off_t   com_fd_tx_lseek(int, off_t, int);
extern int     com_fd_tx_fsync(int);
extern void    com_fd_tx_sync(void);
extern int     com_fd_tx_pipe(int[2]);

ceuta_wrap(int, dup,   com_fd_tx_dup,    int fildes);
ceuta_excl(int, dup2,  dup2,               int oldfd, int newfd);
ceuta_wrap(int, close, com_fd_tx_close,  int fildes);

ceuta_hdrl( #if defined _XOPEN_SOURCE && _XOPEN_SOURCE >= 500 );
ceuta_wrap(ssize_t, pwrite, com_fd_tx_pwrite, int fildes, [siz=nbyte|in] const void *buf, size_t nbyte, off_t off);
ceuta_wrap(ssize_t, pread,  com_fd_tx_pread,  int fildes, [siz=nbyte|out] void *buf, size_t nbyte, off_t off);
ceuta_hdrl( #endif );

ceuta_wrap(ssize_t, write,  com_fd_tx_write,  int fildes, [siz=nbyte|in] const void *buf, size_t nbyte);
ceuta_wrap(ssize_t, read,   com_fd_tx_read,   int fildes, [siz=nbyte|out] void *buf, size_t nbyte);
ceuta_wrap(off_t,   lseek,  com_fd_tx_lseek,  int fildes, off_t offset, int whence);
ceuta_wrap(int,     fsync,  com_fd_tx_fsync,  int fildes);
ceuta_wrap(void,    sync,   com_fd_tx_sync);

extern int
tanger_stm_std_pipe(int pipefd[2])
{
    extern void* com_alloc_tx_malloc(size_t);
    extern void  com_alloc_tx_free(void*);

    tanger_stm_tx_t *tx = tanger_stm_get_tx();
    assert(tx);

    int *fd = com_alloc_tx_malloc(sizeof(pipefd));

    if (!fd) {
        return -1;
    }

    ssize_t res;
    res = com_fd_tx_pipe(fd);

    tanger_stm_storeregion(tx, (uint8_t*)fd, (uintptr_t)(sizeof(pipefd)), (uint8_t*)pipefd);

    com_alloc_tx_free(fd);

    return res;
}

ceuta_hdrl(static int tanger_wrapper_tanger_stm_std_pipe(int[2]) __attribute__ ((weakref("pipe"))););

extern int   com_fs_tx_chdir(const char*);
extern int   com_fs_tx_fchdir(int);
extern char* com_fs_tx_getcwd(char*, size_t);
extern int   com_fs_tx_link(const char*, const char*);
extern int   com_fs_tx_unlink(const char*);

ceuta_wrap(int,   chdir,  com_fs_tx_chdir,  [cstring|in] const char *path);
ceuta_wrap(int,   fchdir, com_fs_tx_fchdir, int fildes);
ceuta_wrap(char*, getcwd, com_fs_tx_getcwd, [siz=size|out] char *buf, size_t size);
ceuta_wrap(int,   link,   com_fs_tx_link,   [cstring|in] const char *oldpath, [cstring|in] const char *newpath);
ceuta_wrap(int,   unlink, com_fs_tx_unlink, [cstring|in] const char *pathname);

ceuta_pure(unsigned, sleep, sleep, unsigned seconds);

ceuta_hdrl(#endif);

