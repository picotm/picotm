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
#include <sys/socket.h>
#include "errcode.h"
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

int
com_fd_exec_socket(struct com_fd *data, int domain, int type, int protocol)
{
    assert(data);

    /* Create socket */

    int sockfd = TEMP_FAILURE_RETRY(socket(domain, type, protocol));

    if (sockfd < 0) {
        return -1;
    }

    /* Update/create fdtx */

    struct fdtx *fdtx = com_fd_get_fdtx(data, sockfd);
    assert(fdtx);

    enum error_code err = fdtx_ref(fdtx, sockfd, 0);

    if (err) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return err;
    }

    /* Inject event */
    if (com_fd_inject(data, ACTION_SOCKET, sockfd, -1) < 0) {
        if (TEMP_FAILURE_RETRY(close(sockfd)) < 0) {
            perror("close");
        }
        return -1;
    }

    return sockfd;
}

int
com_fd_apply_socket(struct com_fd *data, const struct com_fd_event *event, size_t n)
{
    assert(data);
    assert(event || !n);

    return 0;
}

int
com_fd_undo_socket(struct com_fd *data, int fildes, int cookie)
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

