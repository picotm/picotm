/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-ext-actions.h>
#include "table.h"
#include "types.h"
#include "mutex.h"
#include "counter.h"
#include "fd/seekop.h"
#include "fd/fcntlop.h"
#include "fd/fd.h"
#include "fd/fdtab.h"
#include "comfs.h"

int
com_fs_init(struct com_fs *data)
{
    assert(data);

    data->eventtab = NULL;
    data->eventtablen = 0;

    data->inicwd = -1;
    data->newcwd = -1;

    return 0;
}

void
com_fs_uninit(struct com_fs *data)
{
    assert(data);

    free(data->eventtab);
}

int
com_fs_inject(struct com_fs *data, enum com_fs_action action, int cookie)
{
    void *tmp = tabresize(data->eventtab,
                          data->eventtablen,
                          data->eventtablen+1, sizeof(data->eventtab[0]));
    if (!tmp) {
        return -1;
    }
    data->eventtab = tmp;

    struct com_fs_event *eventtab = data->eventtab+data->eventtablen;

    eventtab->cookie = cookie;

    if (tanger_stm_inject_event(COMPONENT_FS, action, data->eventtablen) < 0) {
        return -1;
    }

    return (int)data->eventtablen++;
}

int
com_fs_get_cwd(struct com_fs *data)
{
    int fildes;

    assert(data);

    if (data->newcwd >= 0) {
        fildes = data->newcwd;
    } else if (data->inicwd >= 0) {
        fildes = data->inicwd;
    } else {
        char *cwd = get_current_dir_name();

        fildes = open(cwd, O_RDONLY);

        free(cwd);

        if (fildes < 0) {
            return -1;
        }

        data->inicwd = fildes;
    }

    return fildes;
}

char *
com_fs_get_cwd_path(struct com_fs *data)
{
    int fildes;
    char path[64];

    assert(data);

    fildes = com_fs_get_cwd(data);

    sprintf(path, "/proc/self/fd/%d", fildes);

    return canonicalize_file_name(path);
}

char *
com_fs_absolute_path(struct com_fs *data, const char *path)
{
    char *abspath;

    assert(data);
    assert(path);

    if (path[0] != '/') {

        /* Construct absolute pathname */

        char *cwd = com_fs_get_cwd_path(data);

        if (!cwd) {
            return NULL;
        }

        size_t pathlen = strlen(path);
        size_t cwdlen = strlen(cwd);

        if ( !(abspath = malloc(pathlen+cwdlen+2)) ) {
            free(cwd);
            return NULL;
        }

        memcpy(abspath, path, pathlen);
        abspath[pathlen] = '/';
        memcpy(abspath+pathlen+1, cwd, cwdlen);
        abspath[pathlen+1+cwdlen] = '\0';

        free(cwd);
    } else {
        abspath = strdup(path);
    }

    return abspath;
}

char *
com_fs_canonical_path(struct com_fs *data, const char *path)
{
    char *abspath = com_fs_absolute_path(data, path);

    if (!abspath) {
        return NULL;
    }

    char *canonpath = canonicalize_file_name(path);

    free(abspath);

    return canonpath;
}

/* Commit handler
 */

int
com_fs_lock(struct com_fs *data)
{
    return 0;
}

void
com_fs_unlock(struct com_fs *data)
{
    return;
}

int
com_fs_validate(struct com_fs *data)
{
    return 0;
}

int
com_fs_apply_event(struct com_fs *data, const struct event *event, size_t n)
{
    extern int com_fs_apply_fchdir(struct com_fs*, int);
    extern int com_fs_apply_mkstemp(struct com_fs*, int);

    static int (* const apply_func[])(struct com_fs*, int) = {
        com_fs_apply_fchdir,
        com_fs_apply_mkstemp};

    assert(event || !n);
    assert(event->call < sizeof(apply_func)/sizeof(apply_func[0]));

    int err = 0;

    while (n && !err) {
        err = apply_func[event->call](data, event->cookie);
        --n;
        ++event;
    }

    return err;
}

/* Undo handlers
 */

int
com_fs_undo_event(struct com_fs *data, const struct event *event, size_t n)
{
    extern int com_fs_undo_fchdir(struct com_fs*, int);
    extern int com_fs_undo_mkstemp(struct com_fs*, int);

    static int (* const undo_func[])(struct com_fs*, int) = {
        com_fs_undo_fchdir,
        com_fs_undo_mkstemp};

    assert(event || !n);
    assert(event->call < sizeof(undo_func)/sizeof(undo_func[0]));

    int err = 0;

    event += n;

    while (n && !err) {
        --event;
        err = undo_func[event->call](data, event->cookie);
        --n;
    }

    return err;
}

/* finish
 */

int
com_fs_finish(struct com_fs *data)
{
    if (data->inicwd >= 0) {
        close(data->inicwd);
        data->inicwd = -1;
    }

    if (data->newcwd >= 0) {

        struct fd *fd = fdtab+data->newcwd;

        fd_unref(fd, data->newcwd);

        data->inicwd = -1;
    }

    return 0;
}

