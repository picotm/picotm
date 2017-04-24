/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/sys/socket.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include "fd/comfdtx.h"
#include "picotm/picotm-libc.h"
#include "picotm/sys/socket-tm.h"

PICOTM_EXPORT
int
accept_tx(int socket, struct sockaddr* address, socklen_t* address_len)
{
    if (address) {
        privatize_tx(address_len, sizeof(*address_len), PICOTM_TM_PRIVATIZE_STORE);
        privatize_tx(address, *address_len, PICOTM_TM_PRIVATIZE_LOADSTORE);
    }
    return accept_tm(socket, address, address_len);
}

PICOTM_EXPORT
int
bind_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, PICOTM_TM_PRIVATIZE_LOAD);
    return bind_tm(socket, address, address_len);
}

PICOTM_EXPORT
int
connect_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, PICOTM_TM_PRIVATIZE_LOAD);
    return connect_tm(socket, address, address_len);
}

PICOTM_EXPORT
int
listen_tx(int socket, int backlog)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fd_tx_listen(socket, backlog);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
send_tx(int socket, const void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, PICOTM_TM_PRIVATIZE_LOAD);
    return send_tm(socket, buffer, length, flags);
}

PICOTM_EXPORT
int
shutdown_tx(int socket, int how)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fd_tx_shutdown(socket, how);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
socket_tx(int domain, int type, int protocol)
{
    picotm_libc_save_errno();

    int res;

    do {
        res = com_fd_tx_socket(domain, type, protocol);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
recv_tx(int socket, void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, PICOTM_TM_PRIVATIZE_STORE);
    return recv_tm(socket, buffer, length, flags);
}


