/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/sys/socket-tm.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "fd/comfdtx.h"
#include "picotm/picotm-libc.h"

static bool
perform_recovery(int errno_hint)
{
    enum picotm_libc_error_recovery recovery =
        picotm_libc_get_error_recovery();

    if (recovery == PICOTM_LIBC_ERROR_RECOVERY_FULL) {
        return true;
    }

    return (errno_hint != EAGAIN) && (errno_hint != EWOULDBLOCK);
}

PICOTM_EXPORT
int
accept_tm(int socket, struct sockaddr* address, socklen_t* address_len)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fd_tx_accept(socket, address, address_len);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
bind_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fd_tx_bind(socket, address, address_len);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
connect_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fd_tx_connect(socket, address, address_len);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
send_tm(int socket, const void* buffer, size_t length, int flags)
{
    picotm_libc_save_errno();

    ssize_t res;

    do {
        res = com_fd_tx_send(socket, buffer, length, flags);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
recv_tm(int socket, void* buffer, size_t length, int flags)
{
    picotm_libc_save_errno();

    ssize_t res;

    do {
        res = com_fd_tx_recv(socket, buffer, length, flags);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
