/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vfs_tx.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <unistd.h>
#include "fd/fd.h"
#include "fd/fdtab.h"

enum vfs_tx_cmd {
    CMD_FCHDIR = 0,
    CMD_MKSTEMP,
    LAST_CMD
};

void
vfs_tx_init(struct vfs_tx* self, unsigned long module)
{
    assert(self);

    self->module = module;

    self->eventtab = NULL;
    self->eventtablen = 0;

    self->inicwd = -1;
    self->newcwd = -1;
}

void
vfs_tx_uninit(struct vfs_tx* self)
{
    assert(self);

    picotm_tabfree(self->eventtab);
}

int
vfs_tx_get_cwd(struct vfs_tx* self, struct picotm_error* error)
{
    assert(self);

    if (self->newcwd >= 0) {
        return self->newcwd;
    }

    if (self->inicwd >= 0) {
        return self->inicwd;
    }

    char* cwd = get_current_dir_name();

    int fildes = TEMP_FAILURE_RETRY(open(cwd, O_RDONLY));
    if (fildes < 0) {
        picotm_error_set_errno(error, errno);
        goto err_open;
    }

    self->inicwd = fildes;

    free(cwd);

    return fildes;

err_open:
    free(cwd);
    return -1;
}

char*
vfs_tx_get_cwd_path(struct vfs_tx* self, struct picotm_error* error)
{
    assert(self);

    int fildes = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    char path[64];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fildes);

    char* canonpath = canonicalize_file_name(path);
    if (!canonpath) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    return canonpath;
}

char*
vfs_tx_absolute_path(struct vfs_tx* self, const char* path,
                     struct picotm_error* error)
{
    assert(self);
    assert(path);

    if (path[0] == '/') {
        char* abspath = strdup(path);
        if (!abspath) {
            picotm_error_set_errno(error, errno);
            return NULL;
        }
        return abspath;
    }

    /* Construct absolute pathname */

    char* cwd = vfs_tx_get_cwd_path(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    size_t pathlen = strlen(path);
    size_t cwdlen = strlen(cwd);

    char* abspath = malloc(pathlen + cwdlen + 2 * sizeof(*abspath));
    if (!abspath) {
        picotm_error_set_errno(error, errno);
        goto err_malloc;
    }

    memcpy(abspath, path, pathlen);
    abspath[pathlen] = '/';
    memcpy(abspath + pathlen + 1, cwd, cwdlen);
    abspath[pathlen + 1 + cwdlen] = '\0';

    free(cwd);

    return abspath;

err_malloc:
    free(cwd);
    return NULL;
}

char*
vfs_tx_canonical_path(struct vfs_tx* self, const char *path,
                      struct picotm_error* error)
{
    char* abspath = vfs_tx_absolute_path(self, path, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    char* canonpath = canonicalize_file_name(abspath);
    if (!canonpath) {
        picotm_error_set_errno(error, errno);
        goto err_canonicalize_filename;
    }

    free(abspath);

    return canonpath;

err_canonicalize_filename:
    free(abspath);
    return NULL;
}

static void
append_cmd(struct vfs_tx* self, enum vfs_tx_cmd cmd, int cookie,
           struct picotm_error* error)
{
    void* tmp = picotm_tabresize(self->eventtab,
                                 self->eventtablen,
                                 self->eventtablen + 1,
                                 sizeof(self->eventtab[0]),
                                 error);
    if (picotm_error_is_set(error)) {
        return;
    }
    self->eventtab = tmp;

    struct vfs_tx_event* eventtab = self->eventtab + self->eventtablen;

    eventtab->cookie = cookie;

    picotm_append_event(self->module, cmd, self->eventtablen, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    ++self->eventtablen;
}

/*
 * chmod()
 */

int
vfs_tx_exec_chmod(struct vfs_tx* self, const char* path, mode_t mode,
                  struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = fchmodat(cwd, path, mode, 0);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * fchdir()
 */

int
vfs_tx_exec_fchdir(struct vfs_tx* self, int fildes,
                   struct picotm_error* error)
{
    assert(self);

    /* Reference new directory's file descriptor */

    struct fd* fd = fdtab + fildes;

    fd_ref(fd, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Check file descriptor */

    struct stat buf;
    int res = fstat(fildes, &buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_fstab;
    }

    if (!S_ISDIR(buf.st_mode)) {
        picotm_error_set_errno(error, ENOTDIR);
        goto err_s_isdir;
    }

    if (self->newcwd < 0) {
        /* Inject event */
        append_cmd(self, CMD_FCHDIR, -1, error);
        if (picotm_error_is_set(error)) {
            goto err_append_cmd;
        }
    } else {
        /* Replace old CWD with new CWD */
        struct fd* fd = fdtab + self->newcwd;
        fd_unref(fd);
    }

    self->newcwd = fildes;

    return 0;

err_append_cmd:
err_s_isdir:
err_fstab:
    fd_unref(fd);
    return -1;
}

static void
apply_fchdir(struct vfs_tx* self, int cookie, struct picotm_error* error)
{
    assert(self);

    int res = TEMP_FAILURE_RETRY(fchdir(self->newcwd));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

static void
undo_fchdir(struct vfs_tx* self, int cookie, struct picotm_error* error)
{ }

/*
 * fchmod()
 */

int
vfs_tx_exec_fchmod(struct vfs_tx* self, int fildes, mode_t mode,
                   struct picotm_error* error)
{
    /* reference file descriptor while working on it */

    struct fd* fd = fdtab + fildes;

    fd_ref(fd, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = fchmod(fildes, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_fchmod;
    }

    fd_unref(fd);

    return res;

err_fchmod:
    fd_unref(fd);
    return -1;
}

/*
 * fstat()
 */

int
vfs_tx_exec_fstat(struct vfs_tx* self, int fildes, struct stat* buf,
                  struct picotm_error* error)
{
    /* reference file descriptor while working on it */

    struct fd* fd = fdtab + fildes;

    fd_ref(fd, fildes, 0, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = fstat(fildes, buf);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_fstat;
    }

    fd_unref(fd);

    return res;

err_fstat:
    fd_unref(fd);
    return -1;
}

/*
 * getcwd()
 */

char*
vfs_tx_exec_getcwd(struct vfs_tx* self, char* buf, size_t size,
                   struct picotm_error* error)
{
    /* return transaction-local working directory */

    char* cwd = vfs_tx_get_cwd_path(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    size_t len = strlen(cwd) + sizeof(*cwd);

    if (!(size >= len)) {
        picotm_error_set_errno(error, ERANGE);
        goto err_size_ge_len;
    }

    memcpy(buf, cwd, len);
    free(cwd);

    return buf;

err_size_ge_len:
    free(cwd);
    return NULL;
}

/*
 * link()
 */

int
vfs_tx_exec_link(struct vfs_tx* self, const char* path1, const char* path2,
                 struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = linkat(cwd, path1, cwd, path2, AT_SYMLINK_FOLLOW);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * lstat()
 */

int
vfs_tx_exec_lstat(struct vfs_tx* self, const char* path, struct stat* buf,
                  struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = fstatat(cwd, path, buf, AT_SYMLINK_NOFOLLOW);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * mkdir()
 */

int
vfs_tx_exec_mkdir(struct vfs_tx* self, const char* path, mode_t mode,
                  struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = mkdirat(cwd, path, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * mkfifo()
 */

int
vfs_tx_exec_mkfifo(struct vfs_tx* self, const char* path, mode_t mode,
                   struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = mkfifoat(cwd, path, mode);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * mknod()
 */

int
vfs_tx_exec_mknod(struct vfs_tx* self, const char* path, mode_t mode,
                  dev_t dev, struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = mknodat(cwd, path, mode, dev);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * mkstemp()
 */

int
vfs_tx_exec_mkstemp(struct vfs_tx* self, char* pathname,
                    struct picotm_error* error)
{
    assert(self);
    assert(pathname);

    /* Construct absolute pathname */

    char* abspath = vfs_tx_absolute_path(self, pathname, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    /* Make call */

    int res = mkstemp(abspath);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_mkstemp;
    }
    int fildes = res;

    /* Copy trailing filled XXXXXX back to pathname */
    memcpy(pathname + strlen(pathname) - 6, abspath + strlen(abspath) - 6, 6);

    append_cmd(self, CMD_MKSTEMP, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_append_cmd;
    }

    free(abspath);

    return fildes;

err_append_cmd:
    unlink(abspath);
err_mkstemp:
    free(abspath);
    return -1;
}

static void
apply_mkstemp(struct vfs_tx* self, int cookie, struct picotm_error* error)
{ }

/* Remove temporary file in a quite reliable, but Linux-only, way. This is
 * only possible because it is certain that the transaction created that file
 * initially. Note that there is still a race condition. An attacker could
 * replace the file at 'canonpath' while the process is between stat and
 * unlink.
 */
static void
undo_mkstemp(struct vfs_tx* self, int cookie, struct picotm_error* error)
{
    char path[64];
    int res = snprintf(path, sizeof(path), "/proc/self/fd/%d", cookie);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }

    char* canonpath = canonicalize_file_name(path);

    if (canonpath) {

        struct stat buf[2];

        if (fstat(cookie, buf+0) != -1
            && stat(canonpath, buf+1) != -1
            && buf[0].st_dev == buf[1].st_dev
            && buf[0].st_ino == buf[1].st_ino) {

            if (unlink(canonpath) < 0) {
                perror("unlink");
            }
        }

        free(canonpath);
    } else {
        perror("canonicalize_file_name");
    }

    res = TEMP_FAILURE_RETRY(close(cookie));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

/*
 * stat()
 */

int
vfs_tx_exec_stat(struct vfs_tx* self, const char* path, struct stat* buf,
                 struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = fstatat(cwd, path, buf, 0);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * unlink()
 */

int
vfs_tx_exec_unlink(struct vfs_tx* self, const char* path,
                   struct picotm_error* error)
{
    int cwd = vfs_tx_get_cwd(self, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    int res = unlinkat(cwd, path, 0);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return -1;
    }
    return res;
}

/*
 * Module interfaces
 */

void
vfs_tx_lock(struct vfs_tx* self, struct picotm_error* error)
{ }

void
vfs_tx_unlock(struct vfs_tx* self)
{ }

void
vfs_tx_validate(struct vfs_tx* self, struct picotm_error* error)
{ }

void
vfs_tx_apply_event(struct vfs_tx* self,
                   const struct picotm_event* event,
                   struct picotm_error* error)
{
    static void (* const apply_func[LAST_CMD])(struct vfs_tx*,
                                               int,
                                               struct picotm_error*) = {
        apply_fchdir,
        apply_mkstemp
    };

    assert(event);
    assert(event->call < sizeof(apply_func)/sizeof(apply_func[0]));

    apply_func[event->call](self, event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
vfs_tx_undo_event(struct vfs_tx* self,
                  const struct picotm_event* event,
                  struct picotm_error* error)
{
    static void (* const undo_func[LAST_CMD])(struct vfs_tx*,
                                              int,
                                              struct picotm_error*) = {
        undo_fchdir,
        undo_mkstemp
    };

    assert(event);
    assert(event->call < sizeof(undo_func)/sizeof(undo_func[0]));

    undo_func[event->call](self, event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
vfs_tx_finish(struct vfs_tx* self, struct picotm_error* error)
{
    if (self->inicwd >= 0) {
        int res = TEMP_FAILURE_RETRY(close(self->inicwd));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            return;
        }
        self->inicwd = -1;
    }

    if (self->newcwd >= 0) {
        struct fd* fd = fdtab + self->newcwd;
        fd_unref(fd);
        self->newcwd = -1;
    }
}
