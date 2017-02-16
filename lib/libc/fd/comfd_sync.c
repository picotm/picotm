/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
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
#include "ofdtx.h"
#include "fd.h"
#include "fdtx.h"
#include "comfd.h"

void
com_fd_exec_sync(struct com_fd *data)
{
    assert(data);

    /* Sync */
    sync();

    /* Inject event */
    com_fd_inject(data, ACTION_SYNC, -1, -1);
}

int
com_fd_apply_sync(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    sync();

    return 0;
}

int
com_fd_undo_sync(struct com_fd *data, int fildes, int cookie)
{
    return 0;
}

