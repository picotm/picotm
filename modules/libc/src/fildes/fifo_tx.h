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

#include "picotm/picotm-lib-ref.h"
#include "picotm/picotm-lib-rwstate.h"
#include <sys/types.h>
#include "fifo.h"
#include "file_tx.h"
#include "picotm/picotm-libc.h"

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

    struct picotm_ref16 ref;

    struct file_tx base;

    struct fifo* fifo;

    enum picotm_libc_write_mode wrmode;

    unsigned char* wrbuf;
    size_t         wrbuflen;
    size_t         wrbufsiz;

    struct ioop* wrtab;
    size_t       wrtablen;
    size_t       wrtabsiz;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** State of the local reader/writer locks */
    struct picotm_rwstate rwstate[NUMBER_OF_FIFO_FIELDS];
};

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
 * Acquire a reference on the open file description
 */
void
fifo_tx_ref_or_set_up(struct fifo_tx* self, struct fifo* fifo,
                      struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
fifo_tx_ref(struct fifo_tx* self, struct picotm_error* error);

/**
 * Release reference
 */
void
fifo_tx_unref(struct fifo_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
fifo_tx_holds_ref(struct fifo_tx* self);
