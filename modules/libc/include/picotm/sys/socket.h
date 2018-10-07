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

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include <sys/socket.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sys/socket.h>.
 */

#if defined(PICOTM_LIBC_HAVE_ACCEPT) && PICOTM_LIBC_HAVE_ACCEPT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [accept()][posix::accept].
 *
 * [posix::accept]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/accept.html
 */
int
accept_tx(int socket, struct sockaddr* address, socklen_t* address_len);
#endif

#if defined(PICOTM_LIBC_HAVE_BIND) && PICOTM_LIBC_HAVE_BIND || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [bind()][posix::bind].
 *
 * [posix::bind]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/bind.html
 */
int
bind_tx(int socket, const struct sockaddr* address, socklen_t address_len);
#endif

#if defined(PICOTM_LIBC_HAVE_CONNECT) && PICOTM_LIBC_HAVE_CONNECT || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [connect()][posix::connect].
 *
 * [posix::connect]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/connect.html
 */
int
connect_tx(int socket, const struct sockaddr* address, socklen_t address_len);
#endif

#if defined(PICOTM_LIBC_HAVE_LISTEN) && PICOTM_LIBC_HAVE_LISTEN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [listen()][posix::listen].
 *
 * [posix::listen]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/listen.html
 */
int
listen_tx(int socket, int backlog);
#endif

#if defined(PICOTM_LIBC_HAVE_RECV) && PICOTM_LIBC_HAVE_RECV || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [recv()][posix::recv].
 *
 * [posix::recv]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/recv.html
 */
ssize_t
recv_tx(int socket, void* buffer, size_t length, int flags);
#endif

#if defined(PICOTM_LIBC_HAVE_SEND) && PICOTM_LIBC_HAVE_SEND || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [send()][posix::send].
 *
 * [posix::send]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/send.html
 */
ssize_t
send_tx(int socket, const void* buffer, size_t length, int flags);
#endif

#if defined(PICOTM_LIBC_HAVE_SHUTDOWN) && PICOTM_LIBC_HAVE_SHUTDOWN || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [shutdown()][posix::shutdown].
 *
 * [posix::shutdown]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/shutdown.html
 */
int
shutdown_tx(int socket, int how);
#endif

#if defined(PICOTM_LIBC_HAVE_SOCKET) && PICOTM_LIBC_HAVE_SOCKET || \
    defined(__PICOTM_DOXYGEN)
PICOTM_NOTHROW
/**
 * \ingroup group_libc
 * A transaction-safe implementation of [socket()][posix::socket].
 *
 * [posix::socket]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html
 */
int
socket_tx(int domain, int type, int protocol);
#endif

PICOTM_END_DECLS
