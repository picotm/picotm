/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
#include "types.h"
#include "seekop.h"
#include "rwlock.h"
#include "counter.h"
#include "pgtree.h"
#include "pgtreess.h"
#include "cmap.h"
#include "cmapss.h"
#include "rwlockmap.h"
#include "rwstatemap.h"
#include "pipeop.h"
#include "pipeoptab.h"
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"
#include "ofdtx.h"
#include "fd.h"
#include "fdtab.h"
#include "fdtx.h"
#include "comfd.h"
#include "fs/comfs.h"

int
com_fd_exec_pipe(struct com_fd *data, int pipefd[2])
{
    assert(data);

    /* Create pipe */

    if (TEMP_FAILURE_RETRY(pipe(pipefd)) < 0) {
        return ERR_SYSTEM;
    }

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, pipefd[0]);
    assert(fdtx);

    enum error_code err = fdtx_ref(fdtx, pipefd[0], 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return err;
    }

    fdtx = com_fd_get_fdtx(data, pipefd[1]);
    assert(fdtx);

    err = fdtx_ref(fdtx, pipefd[1], 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return err;
    }

    int cookie = pipeoptab_append(&data->pipeoptab,
                                  &data->pipeoptablen, pipefd);

    /* Inject event */
    if ((cookie >= 0) &&
        (com_fd_inject(data, ACTION_PIPE, 0, cookie) < 0)) {
        if (TEMP_FAILURE_RETRY(close(pipefd[0])) < 0) {
            perror("close");
        }
        if (TEMP_FAILURE_RETRY(close(pipefd[1])) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return 0;
}

int
com_fd_apply_pipe(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_pipe(struct com_fd *data, int fildes, int cookie)
{
    assert(data);

    const struct pipeop *pipeop = data->pipeoptab+cookie;

    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[0])) < 0) {
        return ERR_SYSTEM;
    }
    if (TEMP_FAILURE_RETRY(close(pipeop->pipefd[1])) < 0) {
        return ERR_SYSTEM;
    }

    return 0;
}

