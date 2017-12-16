/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <picotm/compiler.h>
#include <picotm/config/picotm-libc-config.h>
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
 * A transaction-safe implementation of [socket()][posix::socket].
 *
 * [posix::socket]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/socket.html
 */
int
socket_tx(int domain, int type, int protocol);
#endif

PICOTM_END_DECLS
