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
#include <sys/stat.h>
#include <unistd.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-ext-actions.h>
#include "types.h"
#include "mutex.h"
#include "counter.h"
#include "fd/seekop.h"
#include "fd/fcntlop.h"
#include "fd/fd.h"
#include "fd/fdtab.h"
#include "fd/fdtx.h"
#include "comfs.h"

int
com_fs_exec_fchdir(struct com_fs *data, int fildes)
{
    assert(data);

    /* Reference new directory's file descriptor */

    struct fd *fd = fdtab+fildes;

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

    if (data->newcwd < 0) {

        /* Inject event */

        if (com_fs_inject(data, ACTION_FCHDIR, -1) < 0) {
            return -1;
        }
    } else {

        /* Replace old CWD with new CWD */

        struct fd *fd = fdtab+data->newcwd;

        fd_unref(fd, data->newcwd);
    }

    data->newcwd = fildes;

    return 0;
}

int
com_fs_apply_fchdir(struct com_fs *data, int cookie)
{
    assert(data);

    return TEMP_FAILURE_RETRY(fchdir(data->newcwd));
}

int
com_fs_undo_fchdir(struct com_fs *data, int cookie)
{
    assert(data);

    return 0;
}

