/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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
#include "fcntlop.h"
#include "ofdid.h"
#include "ofd.h"
#include "fd.h"
#include "ofdtx.h"
#include "fdtx.h"
#include "comfd.h"

int
com_fd_exec_dup(struct com_fd *data, int fildes, int cloexec)
{
    assert(data);

    /* Reference/validate fdtx for fildes */

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    enum error_code err = fdtx_ref_or_validate(fdtx, fildes, 0);

    if (err) {
        return err;
    }

    /* Duplicate fildes */

    static const int dupcmd[] = {F_DUPFD, F_DUPFD_CLOEXEC};

    long fildes2;

    if (TEMP_FAILURE_RETRY(fcntl(fildes, dupcmd[!!cloexec], &fildes2)) < 0) {
        return ERR_SYSTEM;
    }

    struct fdtx *fdtx2 = com_fd_get_fdtx(data, fildes2);
    assert(fdtx2);

    /* Reference fdtx for fildes2 */

    err = fdtx_ref(fdtx2, fildes2, 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (com_fd_inject(data, ACTION_DUP, fildes2, -1) < 0) {
        if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
            perror("close");
        }
        return ERR_SYSTEM;
    }

    return fildes2;
}

int
com_fd_apply_dup(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_dup(struct com_fd *data, int fildes, int cookie)
{
    assert(data);
    assert(fildes >= 0);
    assert(fildes < MAXNUMFD);

    struct fdtx *fdtx = com_fd_get_fdtx(data, fildes);
    assert(fdtx);

    /* Mark file descriptor to be closed. This works, because dup() occured
       inside transaction. So no other transaction should have access to it. */
    fdtx_signal_close(fdtx);

    return 0;
}

