/*
 * picotm - A system-level transaction manager
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

#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-libc.h"
#include <sys/types.h>
#include "fifo.h"
#include "file_tx.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * Holds transaction-local state for an FIFO.
 */
struct fifo_tx {

    struct file_tx base;

    struct pipebuf_tx* pipebuf_tx;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** State of the local reader/writer locks */
    struct picotm_rwstate rwstate[NUMBER_OF_FIFO_FIELDS];
};

static inline struct fifo_tx*
fifo_tx_of_file_tx(struct file_tx* file_tx)
{
    return picotm_containerof(file_tx, struct fifo_tx, base);
}

/**
 * Init transaction-local open-file-description state
 */
void
fifo_tx_init(struct fifo_tx* self);

/**
 * Uninit state
 */
void
fifo_tx_uninit(struct fifo_tx* self);

/**
 * Prepare after acquiring the first reference on the transaction-local
 * FIFO state.
 */
void
fifo_tx_prepare(struct fifo_tx* self, struct fifo* fifo,
                struct picotm_error* error);

/**
 * Clean up after releasing final reference.
 */
void
fifo_tx_release(struct fifo_tx* self);

void
fifo_tx_try_rdlock_field(struct fifo_tx* self, enum fifo_field field,
                         struct picotm_error* error);

void
fifo_tx_try_wrlock_field(struct fifo_tx* self, enum fifo_field field,
                         struct picotm_error* error);

void
fifo_tx_finish(struct fifo_tx* self);
