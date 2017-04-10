/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/sys/socket-tm.h"
#include "fd/comfdtx.h"

SYSTX_EXPORT
int
accept_tm(int socket, struct sockaddr* address, socklen_t* address_len)
{
    return com_fd_tx_accept(socket, address, address_len);
}

SYSTX_EXPORT
int
bind_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    return com_fd_tx_bind(socket, address, address_len);
}

SYSTX_EXPORT
int
connect_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    return com_fd_tx_connect(socket, address, address_len);
}

SYSTX_EXPORT
ssize_t
send_tm(int socket, const void* buffer, size_t length, int flags)
{
    return com_fd_tx_send(socket, buffer, length, flags);
}

SYSTX_EXPORT
ssize_t
recv_tm(int socket, void* buffer, size_t length, int flags)
{
    return com_fd_tx_recv(socket, buffer, length, flags);
}
