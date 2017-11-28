/* Permission is hereby granted, free of charge, to any person obtaining a
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
