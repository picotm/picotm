/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <sys/socket.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sys/socket.h>.
 */

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [accept()][posix::accept].
 *
 * [posix::accept]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/accept.html
 */
int
accept_tx(int socket, struct sockaddr* address, socklen_t* address_len);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [bind()][posix::bind].
 *
 * [posix::bind]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/bind.html
 */
int
bind_tx(int socket, const struct sockaddr* address, socklen_t address_len);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [connect()][posix::connect].
 *
 * [posix::connect]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/connect.html
 */
int
connect_tx(int socket, const struct sockaddr* address, socklen_t address_len);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [listen()][posix::listen].
 *
 * [posix::listen]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/listen.html
 */
int
listen_tx(int socket, int backlog);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [send()][posix::send].
 *
 * [posix::send]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/send.html
 */
ssize_t
send_tx(int socket, const void* buffer, size_t length, int flags);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [shutdown()][posix::shutdown].
 *
 * [posix::shutdown]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/shutdown.html
 */
int
shutdown_tx(int socket, int how);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [socket()][posix::socket].
 *
 * [posix::socket]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html
 */
int
socket_tx(int domain, int type, int protocol);

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [recv()][posix::recv].
 *
 * [posix::recv]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/recv.html
 */
ssize_t
recv_tx(int socket, void* buffer, size_t length, int flags);

PICOTM_END_DECLS
