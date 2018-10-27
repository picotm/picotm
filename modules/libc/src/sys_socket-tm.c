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

#include "picotm/sys/socket-tm.h"
#include "picotm/picotm-module.h"
#include <errno.h>
#include "error/module.h"
#include "fildes/module.h"

static bool
perform_recovery(int errno_hint)
{
    enum picotm_libc_error_recovery recovery =
        picotm_libc_get_error_recovery();

    if (recovery == PICOTM_LIBC_ERROR_RECOVERY_FULL) {
        return true;
    }

    return (errno_hint != EAGAIN) && (errno_hint != EWOULDBLOCK);
}

#if defined(PICOTM_LIBC_HAVE_ACCEPT) && PICOTM_LIBC_HAVE_ACCEPT
PICOTM_EXPORT
int
accept_tm(int socket, struct sockaddr* address, socklen_t* address_len)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_accept(socket, address, address_len);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_BIND) && PICOTM_LIBC_HAVE_BIND
PICOTM_EXPORT
int
bind_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_bind(socket, address, address_len);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_CONNECT) && PICOTM_LIBC_HAVE_CONNECT
PICOTM_EXPORT
int
connect_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_connect(socket, address, address_len);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_RECV) && PICOTM_LIBC_HAVE_RECV
PICOTM_EXPORT
ssize_t
recv_tm(int socket, void* buffer, size_t length, int flags)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_recv(socket, buffer, length, flags);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif

#if defined(PICOTM_LIBC_HAVE_SEND) && PICOTM_LIBC_HAVE_SEND
PICOTM_EXPORT
ssize_t
send_tm(int socket, const void* buffer, size_t length, int flags)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_send(socket, buffer, length, flags);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
#endif
