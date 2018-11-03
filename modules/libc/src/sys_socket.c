/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "picotm/sys/socket.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-tm.h"
#include "picotm/sys/socket-tm.h"
#include <errno.h>
#include "error/module.h"
#include "fildes/module.h"

#if defined(PICOTM_LIBC_HAVE_ACCEPT) && PICOTM_LIBC_HAVE_ACCEPT
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
#endif

#if defined(PICOTM_LIBC_HAVE_BIND) && PICOTM_LIBC_HAVE_BIND
PICOTM_EXPORT
int
bind_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, PICOTM_TM_PRIVATIZE_LOAD);
    return bind_tm(socket, address, address_len);
}
#endif

#if defined(PICOTM_LIBC_HAVE_CONNECT) && PICOTM_LIBC_HAVE_CONNECT
PICOTM_EXPORT
int
connect_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, PICOTM_TM_PRIVATIZE_LOAD);
    return connect_tm(socket, address, address_len);
}
#endif

#if defined(PICOTM_LIBC_HAVE_LISTEN) && PICOTM_LIBC_HAVE_LISTEN
PICOTM_EXPORT
int
listen_tx(int socket, int backlog)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        int res = fildes_module_listen(socket, backlog, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_RECV) && PICOTM_LIBC_HAVE_RECV
PICOTM_EXPORT
ssize_t
recv_tx(int socket, void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, PICOTM_TM_PRIVATIZE_STORE);
    return recv_tm(socket, buffer, length, flags);
}
#endif

#if defined(PICOTM_LIBC_HAVE_SEND) && PICOTM_LIBC_HAVE_SEND
PICOTM_EXPORT
ssize_t
send_tx(int socket, const void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, PICOTM_TM_PRIVATIZE_LOAD);
    return send_tm(socket, buffer, length, flags);
}
#endif

#if defined(PICOTM_LIBC_HAVE_SHUTDOWN) && PICOTM_LIBC_HAVE_SHUTDOWN
PICOTM_EXPORT
int
shutdown_tx(int socket, int how)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        int res = fildes_module_shutdown(socket, how, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif

#if defined(PICOTM_LIBC_HAVE_SOCKET) && PICOTM_LIBC_HAVE_SOCKET
PICOTM_EXPORT
int
socket_tx(int domain, int type, int protocol)
{
    error_module_save_errno();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        int res = fildes_module_socket(domain, type, protocol, &error);
        if (!picotm_error_is_set(&error)) {
            return res;
        }
        picotm_recover_from_error(&error);
    } while (true);
}
#endif
