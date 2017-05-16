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
 * Variant of accept_tx() that operates on transactional memory.
 */
int
accept_tm(int socket, struct sockaddr* address, socklen_t* address_len);

PICOTM_NOTHROW
/**
 * Variant of bind_tx() that operates on transactional memory.
 */
int
bind_tm(int socket, const struct sockaddr* address, socklen_t address_len);

PICOTM_NOTHROW
/**
 * Variant of connect_tx() that operates on transactional memory.
 */
int
connect_tm(int socket, const struct sockaddr* address, socklen_t address_len);

PICOTM_NOTHROW
/**
 * Variant of send_tx() that operates on transactional memory.
 */
ssize_t
send_tm(int socket, const void* buffer, size_t length, int flags);

PICOTM_NOTHROW
/**
 * Variant of recv_tx() that operates on transactional memory.
 */
ssize_t
recv_tm(int socket, void* buffer, size_t length, int flags);

PICOTM_END_DECLS
