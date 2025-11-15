/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2019  Thomas Zimmermann
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

#include "picotm/picotm-lib-ref.h"
#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-libc.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "file_tx.h"
#include "socket.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
struct sockbuf;

/**
 * Holds transaction-local reads and writes for an open file description
 */
struct socket_tx {

    struct file_tx base;

    struct sockbuf_tx* sockbuf_tx;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_SOCKET_FIELDS];
};

static inline struct socket_tx*
socket_tx_of_file_tx(struct file_tx* file_tx)
{
    return picotm_containerof(file_tx, struct socket_tx, base);
}

/**
 * Init transaction-local open-file-description state
 */
void
socket_tx_init(struct socket_tx* self);

/**
 * Uninit state
 */
void
socket_tx_uninit(struct socket_tx* self);

/**
 * Preapre after acquiring the first reference on the transaction-local
 * socket state.
 */
void
socket_tx_prepare(struct socket_tx* self, struct socket* socket,
                  struct picotm_error* error);

/**
 * Clean up after releasing final reference
 */
void
socket_tx_release(struct socket_tx* self);

void
socket_tx_try_rdlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error);

void
socket_tx_try_wrlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error);

void
socket_tx_finish(struct socket_tx* self);
