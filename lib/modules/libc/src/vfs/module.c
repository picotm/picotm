/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
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

struct vfs_module {
    bool          is_initialized;
    struct vfs_tx tx;
};

static int
lock_cb(void* data)
{
    struct vfs_module* module = data;

    return vfs_tx_lock(&module->tx);
}

static int
unlock_cb(void* data)
{
    struct vfs_module* module = data;

    vfs_tx_unlock(&module->tx);

    return 0;
}

static int
validate_cb(void* data, int noundo)
{
    struct vfs_module* module = data;

    return vfs_tx_validate(&module->tx);
}

static int
apply_event_cb(const struct event* event, size_t n, void* data)
{
    struct vfs_module* module = data;

    return vfs_tx_apply_event(&module->tx, event, n);
}

static int
undo_event_cb(const struct event* event, size_t n, void* data)
{
    struct vfs_module* module = data;

    return vfs_tx_undo_event(&module->tx, event, n);
}

static int
finish_cb(void* data)
{
    struct vfs_module* module = data;

    vfs_tx_finish(&module->tx);

    return 0;
}

static int
uninit_cb(void* data)
{
    struct vfs_module* module = data;

    vfs_tx_uninit(&module->tx);
    module->is_initialized = false;

    return 0;
}

static struct vfs_tx*
get_vfs_tx(void)
{
    static __thread struct vfs_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    }

    long res = picotm_register_module(lock_cb,
                                      unlock_cb,
                                      validate_cb,
                                      apply_event_cb,
                                      undo_event_cb,
                                      NULL,
                                      NULL,
                                      finish_cb,
                                      uninit_cb,
                                      &t_module);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = vfs_tx_init(&t_module.tx, module);
    if (res < 0) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.tx;
}

int
vfs_module_chdir(const char* path)
{
    int fildes = fd_module_open(path, O_RDONLY, 0);
    if (fildes < 0) {
        return -1;
    }

    return vfs_module_fchdir(fildes);
}

int
vfs_module_chmod(const char* path, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return fchmodat(vfs_tx_get_cwd(vfs_tx), path, mode, 0);
}

int
vfs_module_fchdir(int fildes)
{
    int res;

    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    do {
        res = vfs_tx_exec_fchdir(vfs_tx, fildes);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_restart();
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
vfs_module_fchmod(int fildes, mode_t mode)
{
    /* reference file descriptor while working on it */

    struct fd* fd = fdtab + fildes;

    int err = fd_ref(fd, fildes, 0);
    if (err) {
        return err;
    }

    int res = fchmod(fildes, mode);

    fd_unref(fd, fildes);

    return res;
}

int
vfs_module_fstat(int fildes, struct stat* buf)
{
    /* reference file descriptor while working on it */

    struct fd* fd = fdtab + fildes;

    int err = fd_ref(fd, fildes, 0);
    if (err) {
        return err;
    }

    int res = fstat(fildes, buf);

    fd_unref(fd, fildes);

    return res;
}

char*
vfs_module_getcwd(char* buf, size_t size)
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
vfs_module_getcwd_fildes()
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return vfs_tx_get_cwd(vfs_tx);
}

int
vfs_module_link(const char* path1, const char* path2)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    int cwd = vfs_tx_get_cwd(vfs_tx);

    return linkat(cwd, path1, cwd, path2, AT_SYMLINK_FOLLOW);
}

int
vfs_module_lstat(const char* path, struct stat* buf)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return fstatat(vfs_tx_get_cwd(vfs_tx), path, buf, AT_SYMLINK_NOFOLLOW);
}

int
vfs_module_mkdir(const char* path, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return mkdirat(vfs_tx_get_cwd(vfs_tx), path, mode);
}

int
vfs_module_mkfifo(const char* path, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return mkfifoat(vfs_tx_get_cwd(vfs_tx), path, mode);
}

int
vfs_module_mknod(const char* path, mode_t mode, dev_t dev)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return mknodat(vfs_tx_get_cwd(vfs_tx), path, mode, dev);
}

int
vfs_module_mkstemp(char* template)
{
    int res;

    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    do {
        res = vfs_tx_exec_mkstemp(vfs_tx, template);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                picotm_restart();
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
vfs_module_stat(const char* path, struct stat* buf)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return fstatat(vfs_tx_get_cwd(vfs_tx), path, buf, 0);
}

int
vfs_module_unlink(const char* path)
{
    struct vfs_tx* vfs_tx = get_vfs_tx();
    assert(vfs_tx);

    return unlinkat(vfs_tx_get_cwd(vfs_tx), path, 0);
}
