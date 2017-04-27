/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comfstx.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm.h>
#include <picotm/picotm-module.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errcode.h"
#include "fd/fd.h"
#include "fd/fdtab.h"
#include "fd/module.h"
#include "vfs_tx.h"

static int
com_fs_tx_lock(void *data)
{
    return vfs_tx_lock(data);
}

static int
com_fs_tx_unlock(void *data)
{
    vfs_tx_unlock(data);

    return 0;
}

static int
com_fs_tx_validate(void *data, int noundo)
{
    return vfs_tx_validate(data);
}

static int
com_fs_tx_apply_event(const struct event *event, size_t n, void *data)
{
    return vfs_tx_apply_event(data, event, n);
}

static int
com_fs_tx_undo_event(const struct event *event, size_t n, void *data)
{
    return vfs_tx_undo_event(data, event, n);
}

static int
com_fs_tx_finish(void *data)
{
    vfs_tx_finish(data);

    return 0;
}

static int
com_fs_tx_uninit(void *data)
{
    vfs_tx_uninit(data);

    return 0;
}

static struct vfs_tx*
get_vfs_tx(void)
{
    static __thread struct {
        bool          is_initialized;
        struct vfs_tx tx;
    } t_com_fs;

    if (t_com_fs.is_initialized) {
        return &t_com_fs.tx;
    }

    long res = picotm_register_module(com_fs_tx_lock,
                                      com_fs_tx_unlock,
                                      com_fs_tx_validate,
                                      com_fs_tx_apply_event,
                                      com_fs_tx_undo_event,
                                      NULL,
                                      NULL,
                                      com_fs_tx_finish,
                                      com_fs_tx_uninit,
                                      &t_com_fs.tx);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = vfs_tx_init(&t_com_fs.tx, module);
    if (res < 0) {
        return NULL;
    }

    t_com_fs.is_initialized = true;

    return &t_com_fs.tx;
}

int
com_fs_tx_fchdir(int fildes)
{
    int res;

    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    do {
        res = vfs_tx_exec_fchdir(vfs_tx, fildes);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fs_tx_mkstemp(char *pathname)
{
    int res;

    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    do {
        res = vfs_tx_exec_mkstemp(vfs_tx, pathname);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_abort();
                break;
            case ERR_NOUNDO:
                picotm_irrevocable();
                break;
            default:
                break;
        }
    } while (res == ERR_NOUNDO);

    return res;
}

int
com_fs_tx_chdir(const char *path)
{
    assert(path);

    int fildes = fd_module_open(path, O_RDONLY, 0);

    if (fildes < 0) {
        return -1;
    }

    return com_fs_tx_fchdir(fildes);
}

int
com_fs_tx_chmod(const char *pathname, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return fchmodat(vfs_tx_get_cwd(vfs_tx), pathname, mode, 0);
}

int
com_fs_tx_fchmod(int fildes, mode_t mode)
{
    /* reference file descriptor while working on it */

    struct fd *fd = fdtab+fildes;

    int err = fd_ref(fd, fildes, 0);

    if (err) {
        return err;
    }

    int res = fchmod(fildes, mode);

    fd_unref(fd, fildes);

    return res;
}

int
com_fs_tx_fstat(int fildes, struct stat *buf)
{
    /* reference file descriptor while working on it */

    struct fd *fd = fdtab+fildes;

    int err = fd_ref(fd, fildes, 0);

    if (err) {
        return err;
    }

    int res = fstat(fildes, buf);

    fd_unref(fd, fildes);

    return res;
}

char *
com_fs_tx_getcwd(char *buf, size_t size)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    if (!size) {
        errno = EINVAL;
        return NULL;
    }

    /* return transaction-local working directory */

    char* cwd = vfs_tx_get_cwd_path(vfs_tx);
    if (!cwd) {
        return NULL;
    }

    size_t len = strlen(cwd) + sizeof(*cwd);

    if (!(size >= len)) {
        errno = ERANGE;
        goto err_size_ge_len;
    }

    memcpy(buf, cwd, len);
    free(cwd);

    return buf;

err_size_ge_len:
    free(cwd);
    return NULL;
}

int
com_fs_tx_getcwd_fildes()
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return vfs_tx_get_cwd(vfs_tx);
}

int
com_fs_tx_link(const char *oldpath, const char *newpath)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    int cwd = vfs_tx_get_cwd(vfs_tx);

    return linkat(cwd, oldpath, cwd, newpath, AT_SYMLINK_FOLLOW);
}

int
com_fs_tx_lstat(const char *path, struct stat *buf)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return fstatat(vfs_tx_get_cwd(vfs_tx), path, buf, AT_SYMLINK_NOFOLLOW);
}

int
com_fs_tx_mkdir(const char *pathname, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return mkdirat(vfs_tx_get_cwd(vfs_tx), pathname, mode);
}

int
com_fs_tx_mkfifo(const char *path, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return mkfifoat(vfs_tx_get_cwd(vfs_tx), path, mode);
}

int
com_fs_tx_mknod(const char *path, mode_t mode, dev_t dev)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return mknodat(vfs_tx_get_cwd(vfs_tx), path, mode, dev);
}

int
com_fs_tx_stat(const char *path, struct stat *buf)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return fstatat(vfs_tx_get_cwd(vfs_tx), path, buf, 0);
}

int
com_fs_tx_unlink(const char *pathname)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return unlinkat(vfs_tx_get_cwd(vfs_tx), pathname, 0);
}

