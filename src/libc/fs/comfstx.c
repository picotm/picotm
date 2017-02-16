/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
#include "types.h"
#include "counter.h"
#include "fd/fcntlop.h"
#include "fd/fd.h"
#include "fd/fdtab.h"
#include "comfs.h"
#include "comfstx.h"

static int
com_fs_tx_lock(void *data)
{
    return com_fs_lock(data);
}

static int
com_fs_tx_unlock(void *data)
{
    com_fs_unlock(data);

    return 0;
}

static int
com_fs_tx_validate(void *data, int noundo)
{
    return com_fs_validate(data);
}

static int
com_fs_tx_apply_event(const struct event *event, size_t n, void *data)
{
    return com_fs_apply_event(data, event, n);
}

static int
com_fs_tx_undo_event(const struct event *event, size_t n, void *data)
{
    return com_fs_undo_event(data, event, n);
}

static int
com_fs_tx_finish(void *data)
{
    com_fs_finish(data);

    return 0;
}

static int
com_fs_tx_uninit(void *data)
{
    com_fs_uninit(data);

    free(data);

    return 0;
}

struct com_fs *
com_fs_tx_aquire_data()
{
    struct com_fs *data = tanger_stm_get_component_data(COMPONENT_FS);

    if (!data) {
        int res;

        data = malloc(sizeof(data[0]));
        assert(data);

        if (com_fs_init(data) < 0) {
            free(data);
            return NULL;
        }

        res = tanger_stm_register_component(COMPONENT_FS,
                                            com_fs_tx_lock,
                                            com_fs_tx_unlock,
                                            com_fs_tx_validate,
                                            com_fs_tx_apply_event,
                                            com_fs_tx_undo_event,
                                            NULL,
                                            NULL,
                                            com_fs_tx_finish,
                                            com_fs_tx_uninit,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL,
                                            data);

        if (res < 0) {
            com_fs_uninit(data);
            free(data);
            return NULL;
        }
    }

    return data;
}

int
com_fs_tx_fchdir(int fildes)
{
    extern int com_fs_exec_fchdir(struct com_fs*, int);

    int res;

    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    do {
        res = com_fs_exec_fchdir(comfs, fildes);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                tanger_stm_abort_self(tanger_stm_get_tx());
                break;
            case ERR_NOUNDO:
                {
                    enum error_code err
                        = tanger_stm_go_noundo(tanger_stm_get_tx());

                    if (err) {
                        if (err == ERR_CONFLICT) {
                            tanger_stm_abort_self(tanger_stm_get_tx());
                        } else {
                            abort();
                        }
                    }
                }
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
    extern int com_fs_exec_mkstemp(struct com_fs*, char*);

    int res;

    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    do {
        res = com_fs_exec_mkstemp(comfs, pathname);

        switch (res) {
            case ERR_CONFLICT:
            case ERR_PEERABORT:
                tanger_stm_abort_self(tanger_stm_get_tx());
                break;
            case ERR_NOUNDO:
                {
                    enum error_code err
                        = tanger_stm_go_noundo(tanger_stm_get_tx());

                    if (err) {
                        if (err == ERR_CONFLICT) {
                            tanger_stm_abort_self(tanger_stm_get_tx());
                        } else {
                            abort();
                        }
                    }
                }
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
    extern int com_fd_tx_open(const char*, int, mode_t);
    extern int com_fs_tx_fchdir(int);

    assert(path);

    int fildes = TEMP_FAILURE_RETRY(com_fd_tx_open(path, O_RDONLY, 0));

    if (fildes < 0) {
        return -1;
    }

    return com_fs_tx_fchdir(fildes);
}

int
com_fs_tx_chmod(const char *pathname, mode_t mode)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return fchmodat(com_fs_get_cwd(comfs), pathname, mode, 0);
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
    extern void* com_alloc_tx_malloc(size_t);

    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    if (buf && !size) {
        errno = EINVAL;
        return NULL;
    }

    /* get transaction-local working directory */

    char *cwd = com_fs_get_cwd_path(comfs);

    if (!cwd) {
        return NULL;
    }

    size_t len = 1+strlen(cwd);

    if (buf) {
        if (size < len) {
            free(cwd);
            errno = ERANGE;
            return NULL;
        }
    } else {
        buf = com_alloc_tx_malloc(len);

        if (!buf) {
            free(cwd);
            return NULL;
        }
    }

    memcpy(buf, cwd, len);

    free(cwd);

    return buf;
}

int
com_fs_tx_link(const char *oldpath, const char *newpath)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return linkat(com_fs_get_cwd(comfs), oldpath,
                  com_fs_get_cwd(comfs), newpath, AT_SYMLINK_FOLLOW);
}

int
com_fs_tx_lstat(const char *path, struct stat *buf)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return fstatat(com_fs_get_cwd(comfs), path, buf, AT_SYMLINK_NOFOLLOW);
}

int
com_fs_tx_mkdir(const char *pathname, mode_t mode)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return mkdirat(com_fs_get_cwd(comfs), pathname, mode);
}

int
com_fs_tx_mkfifo(const char *path, mode_t mode)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return mkfifoat(com_fs_get_cwd(comfs), path, mode);
}

int
com_fs_tx_mknod(const char *path, mode_t mode, dev_t dev)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return mknodat(com_fs_get_cwd(comfs), path, mode, dev);
}

int
com_fs_tx_stat(const char *path, struct stat *buf)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return fstatat(com_fs_get_cwd(comfs), path, buf, 0);
}

int
com_fs_tx_unlink(const char *pathname)
{
    struct com_fs *comfs = com_fs_tx_aquire_data();
    assert(comfs);

    return unlinkat(com_fs_get_cwd(comfs), pathname, 0);
}

