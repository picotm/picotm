/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2020  Thomas Zimmermann
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
#include <sys/types.h>
#include "picotm/picotm-libc.h"
#include "file_tx.h"
#include "regfile.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * Holds transaction-local reads and writes for a regular file.
 */
struct regfile_tx {

    struct file_tx base;

    struct seekbuf_tx* seekbuf_tx;

    struct fchmodop* fchmodtab;
    size_t           fchmodtablen;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    struct seekop* seektab;
    size_t         seektablen;

    /* Transaction-local file offset */
    off_t offset;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_REGFILE_FIELDS];
};

static inline struct regfile_tx*
regfile_tx_of_file_tx(struct file_tx* file_tx)
{
    return picotm_containerof(file_tx, struct regfile_tx, base);
}

/**
 * Init transaction-local open-file-description state
 */
void
regfile_tx_init(struct regfile_tx* self);

/**
 * Uninit state
 */
void
regfile_tx_uninit(struct regfile_tx* self);

/**
 * Prepare after acquiring the first reference on the transaction-local
 * regular-file state.
 */
void
regfile_tx_prepare(struct regfile_tx* self, struct regfile* regfile,
                   struct picotm_error* error);

/**
 * Clean up after releasing reference.
 */
void
regfile_tx_release(struct regfile_tx* self);

void
regfile_tx_try_rdlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error);

void
regfile_tx_try_wrlock_field(struct regfile_tx* self, enum regfile_field field,
                            struct picotm_error* error);

void
regfile_tx_finish(struct regfile_tx* self);

off_t
regfile_tx_get_file_offset(struct regfile_tx* self, int fildes,
                           struct picotm_error* error);

void
regfile_tx_set_file_offset(struct regfile_tx* self, off_t offset,
                           struct picotm_error* error);
