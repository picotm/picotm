/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <sys/socket.h>

PICOTM_NOTHROW
int
accept_tx(int socket, struct sockaddr* address, socklen_t* address_len);

PICOTM_NOTHROW
int
bind_tx(int socket, const struct sockaddr* address, socklen_t address_len);

PICOTM_NOTHROW
int
connect_tx(int socket, const struct sockaddr* address, socklen_t address_len);

PICOTM_NOTHROW
int
listen_tx(int socket, int backlog);

PICOTM_NOTHROW
ssize_t
send_tx(int socket, const void* buffer, size_t length, int flags);

PICOTM_NOTHROW
int
shutdown_tx(int socket, int how);

PICOTM_NOTHROW
int
socket_tx(int domain, int type, int protocol);

PICOTM_NOTHROW
ssize_t
recv_tx(int socket, void* buffer, size_t length, int flags);
