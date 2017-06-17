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
#include <unistd.h>
#include "fd/fd.h"
#include "fd/fdtab.h"
#include "fd/module.h"
#include "vfs_tx.h"

struct vfs_module {
    bool          is_initialized;
    struct vfs_tx tx;
};

static void
lock_cb(void* data, struct picotm_error* error)
{
    struct vfs_module* module = data;

    vfs_tx_lock(&module->tx, error);
}

static void
unlock_cb(void* data, struct picotm_error* error)
{
    struct vfs_module* module = data;

    vfs_tx_unlock(&module->tx);
}

static bool
is_valid_cb(void* data, int noundo, struct picotm_error* error)
{
    struct vfs_module* module = data;

    vfs_tx_validate(&module->tx, error);
    return !picotm_error_is_set(error);
}

static void
apply_event_cb(const struct picotm_event* event, void* data,
               struct picotm_error* error)
{
    struct vfs_module* module = data;

    vfs_tx_apply_event(&module->tx, event, error);
}

static void
undo_event_cb(const struct picotm_event* event, void* data,
              struct picotm_error* error)
{
    struct vfs_module* module = data;

    vfs_tx_undo_event(&module->tx, event, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct vfs_module* module = data;

    vfs_tx_finish(&module->tx, error);
}

static void
uninit_cb(void* data)
{
    struct vfs_module* module = data;

    vfs_tx_uninit(&module->tx);
    module->is_initialized = false;
}

static struct vfs_tx*
get_vfs_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct vfs_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(lock_cb,
                                                  unlock_cb,
                                                  is_valid_cb,
                                                  NULL, NULL,
                                                  apply_event_cb,
                                                  undo_event_cb,
                                                  NULL, NULL,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    vfs_tx_init(&t_module.tx, module);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct vfs_tx*
get_non_null_vfs_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct vfs_tx* vfs_tx = get_vfs_tx(true, &error);

        if (!picotm_error_is_set(&error)) {
            assert(vfs_tx);
            return vfs_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
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
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_chmod(vfs_tx, path, mode, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_fchdir(int fildes)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_fchdir(vfs_tx, fildes, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_fchmod(int fildes, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_fchmod(vfs_tx, fildes, mode, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_fstat(int fildes, struct stat* buf)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_fstat(vfs_tx, fildes, buf, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

char*
vfs_module_getcwd(char* buf, size_t size)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        char* cwd = vfs_tx_exec_getcwd(vfs_tx, buf, size, &error);

        if (!picotm_error_is_set(&error)) {
            return cwd;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_getcwd_fildes()
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int fildes = vfs_tx_get_cwd(vfs_tx, &error);

        if (!picotm_error_is_set(&error)) {
            return fildes;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_link(const char* path1, const char* path2)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_link(vfs_tx, path1, path2, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_lstat(const char* path, struct stat* buf)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_lstat(vfs_tx, path, buf, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_mkdir(const char* path, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_mkdir(vfs_tx, path, mode, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_mkfifo(const char* path, mode_t mode)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_mkfifo(vfs_tx, path, mode, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_mknod(const char* path, mode_t mode, dev_t dev)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int res = vfs_tx_exec_mknod(vfs_tx, path, mode, dev, &error);

        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_mkstemp(char* template)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int fildes = vfs_tx_exec_mkstemp(vfs_tx, template, &error);

        if (!picotm_error_is_set(&error)) {
            return fildes;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_stat(const char* path, struct stat* buf)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int fildes = vfs_tx_exec_stat(vfs_tx, path, buf, &error);

        if (!picotm_error_is_set(&error)) {
            return fildes;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

int
vfs_module_unlink(const char* path)
{
    struct vfs_tx* vfs_tx = get_non_null_vfs_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        int fildes = vfs_tx_exec_unlink(vfs_tx, path, &error);

        if (!picotm_error_is_set(&error)) {
            return fildes;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
