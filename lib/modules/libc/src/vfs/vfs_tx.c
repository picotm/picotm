/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vfs_tx.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <unistd.h>
#include "errcode.h"
#include "fd/fd.h"
#include "fd/fdtab.h"

enum vfs_tx_cmd {
    CMD_FCHDIR = 0,
    CMD_MKSTEMP,
    LAST_CMD
};

int
vfs_tx_init(struct vfs_tx* self, unsigned long module)
{
    assert(self);

    self->module = module;

    self->eventtab = NULL;
    self->eventtablen = 0;

    self->inicwd = -1;
    self->newcwd = -1;

    return 0;
}

void
vfs_tx_uninit(struct vfs_tx* self)
{
    assert(self);

    picotm_tabfree(self->eventtab);
}

int
vfs_tx_get_cwd(struct vfs_tx* self)
{
    int fildes;

    assert(self);

    if (self->newcwd >= 0) {
        fildes = self->newcwd;
    } else if (self->inicwd >= 0) {
        fildes = self->inicwd;
    } else {
        char* cwd = get_current_dir_name();

        fildes = open(cwd, O_RDONLY);

        free(cwd);

        if (fildes < 0) {
            return -1;
        }

        self->inicwd = fildes;
    }

    return fildes;
}

char*
vfs_tx_get_cwd_path(struct vfs_tx* self)
{
    assert(self);

    int fildes = vfs_tx_get_cwd(self);

    char path[64];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fildes);

    return canonicalize_file_name(path);
}

char*
vfs_tx_absolute_path(struct vfs_tx* self, const char* path)
{
    assert(self);
    assert(path);

    if (path[0] == '/') {
        return strdup(path);
    }

    /* Construct absolute pathname */

    char* cwd = vfs_tx_get_cwd_path(self);
    if (!cwd) {
        return NULL;
    }

    size_t pathlen = strlen(path);
    size_t cwdlen = strlen(cwd);

    char* abspath = malloc(pathlen + cwdlen + 2 * sizeof(*abspath));
    if (!abspath) {
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
vfs_tx_canonical_path(struct vfs_tx* self, const char *path)
{
    char* abspath = vfs_tx_absolute_path(self, path);
    if (!abspath) {
        return NULL;
    }

    char* canonpath = canonicalize_file_name(path);

    free(abspath);

    return canonpath;
}

static int
append_cmd(struct vfs_tx* self, enum vfs_tx_cmd cmd, int cookie)
{
    void* tmp = picotm_tabresize(self->eventtab,
                                 self->eventtablen,
                                 self->eventtablen + 1,
                                 sizeof(self->eventtab[0]));
    if (!tmp) {
        return -1;
    }
    self->eventtab = tmp;

    struct vfs_tx_event* eventtab = self->eventtab + self->eventtablen;

    eventtab->cookie = cookie;

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_append_event(self->module, cmd, self->eventtablen, &error);
    if (picotm_error_is_set(&error)) {
        return -1;
    }

    return (int)self->eventtablen++;
}

/*
 * fchdir()
 */

int
vfs_tx_exec_fchdir(struct vfs_tx* self, int fildes)
{
    assert(self);

    /* Reference new directory's file descriptor */

    struct fd* fd = fdtab + fildes;

    int err = fd_ref(fd, fildes, 0);
    if (err) {
        return err;
    }

    /* Check file descriptor */

    struct stat buf;

    if ((fstat(fildes, &buf) < 0) || (!S_ISDIR(buf.st_mode))) {
        fd_unref(fd, fildes);
        return ERR_SYSTEM;
    }

    if (self->newcwd < 0) {
        /* Inject event */
        if (append_cmd(self, CMD_FCHDIR, -1) < 0) {
            return -1;
        }
    } else {
        /* Replace old CWD with new CWD */
        struct fd* fd = fdtab + self->newcwd;
        fd_unref(fd, self->newcwd);
    }

    self->newcwd = fildes;

    return 0;
}

void
apply_fchdir(struct vfs_tx* self, int cookie, struct picotm_error* error)
{
    assert(self);

    int res = TEMP_FAILURE_RETRY(fchdir(self->newcwd));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
}

void
undo_fchdir(struct vfs_tx* self, int cookie, struct picotm_error* error)
{ }

/*
 * mkstemp()
 */

int
vfs_tx_exec_mkstemp(struct vfs_tx* self, char* pathname)
{
    assert(self);
    assert(pathname);

    /* Construct absolute pathname */

    char* abspath = vfs_tx_absolute_path(self, pathname);
    if (!abspath) {
        return -1;
    }

    /* Make call */

    int res = mkstemp(abspath);
    if (res < 0) {
        goto err_mkstemp;
    }
    int fildes = res;

    /* Copy trailing filled XXXXXX back to pathname */
    memcpy(pathname + strlen(pathname) - 6, abspath + strlen(abspath) - 6, 6);

    res = append_cmd(self, CMD_MKSTEMP, fildes);
    if (res < 0) {
        goto err_append_cmd;
    }

    free(abspath);

    return fildes;

err_append_cmd:
    unlink(abspath);
err_mkstemp:
    free(abspath);
    return res;
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
    snprintf(path, sizeof(path), "/proc/self/fd/%d", cookie);

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

    int res = TEMP_FAILURE_RETRY(close(cookie));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return;
    }
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
                   const struct picotm_event* event, size_t n,
                   struct picotm_error* error)
{
    static void (* const apply_func[LAST_CMD])(struct vfs_tx*,
                                               int,
                                               struct picotm_error*) = {
        apply_fchdir,
        apply_mkstemp
    };

    assert(event || !n);
    assert(event->call < sizeof(apply_func)/sizeof(apply_func[0]));

    while (n) {
        apply_func[event->call](self, event->cookie, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        --n;
        ++event;
    }
}

void
vfs_tx_undo_event(struct vfs_tx* self,
                  const struct picotm_event* event, size_t n,
                  struct picotm_error* error)
{
    static void (* const undo_func[LAST_CMD])(struct vfs_tx*,
                                              int,
                                              struct picotm_error*) = {
        undo_fchdir,
        undo_mkstemp
    };

    assert(event || !n);
    assert(event->call < sizeof(undo_func)/sizeof(undo_func[0]));

    event += n;

    while (n) {
        --event;
        undo_func[event->call](self, event->cookie, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        --n;
    }
}

void
vfs_tx_finish(struct vfs_tx* self, struct picotm_error* error)
{
    if (self->inicwd >= 0) {
        TEMP_FAILURE_RETRY(close(self->inicwd));
        self->inicwd = -1;
    }

    if (self->newcwd >= 0) {
        struct fd* fd = fdtab + self->newcwd;
        fd_unref(fd, self->newcwd);
        self->newcwd = -1;
    }
}
