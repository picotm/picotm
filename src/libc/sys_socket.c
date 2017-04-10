/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/sys/socket.h"
#include <systx/systx-tm.h>
#include "fd/comfdtx.h"
#include "systx/sys/socket-tm.h"

SYSTX_EXPORT
int
accept_tx(int socket, struct sockaddr* address, socklen_t* address_len)
{
    if (address) {
        privatize_tx(address_len, sizeof(*address_len), SYSTX_TM_PRIVATIZE_STORE);
        privatize_tx(address, *address_len, SYSTX_TM_PRIVATIZE_LOADSTORE);
    }
    return accept_tm(socket, address, address_len);
}

SYSTX_EXPORT
int
bind_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, SYSTX_TM_PRIVATIZE_LOAD);
    return bind_tm(socket, address, address_len);
}

SYSTX_EXPORT
int
connect_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, SYSTX_TM_PRIVATIZE_LOAD);
    return connect_tm(socket, address, address_len);
}

SYSTX_EXPORT
int
listen_tx(int socket, int backlog)
{
    return com_fd_tx_listen(socket, backlog);
}

SYSTX_EXPORT
ssize_t
send_tx(int socket, const void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, SYSTX_TM_PRIVATIZE_LOAD);
    return send_tm(socket, buffer, length, flags);
}

SYSTX_EXPORT
int
shutdown_tx(int socket, int how)
{
    return com_fd_tx_shutdown(socket, how);
}

SYSTX_EXPORT
int
socket_tx(int domain, int type, int protocol)
{
    return com_fd_tx_socket(domain, type, protocol);
}

SYSTX_EXPORT
ssize_t
recv_tx(int socket, void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, SYSTX_TM_PRIVATIZE_STORE);
    return recv_tm(socket, buffer, length, flags);
}


