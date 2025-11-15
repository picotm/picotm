/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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
#include "dir.h"
#include "file_tx.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct fchmodop;
struct picotm_error;

/**
 * Holds transaction-local state for a directory.
 */
struct dir_tx {

    struct file_tx base;

    enum picotm_libc_write_mode wrmode;

    struct fchmodop* fchmodtab;
    size_t           fchmodtablen;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** State of the local locks */
    struct picotm_rwstate rwstate[NUMBER_OF_DIR_FIELDS];
};

static inline struct dir_tx*
dir_tx_of_file_tx(struct file_tx* file_tx)
{
    return picotm_containerof(file_tx, struct dir_tx, base);
}

/**
 * Initialize transaction-local directory.
 * \param   self    The instance of `struct dir` to initialize.
 */
void
dir_tx_init(struct dir_tx* self);

/**
 * Uninitialize transaction-local directory.
 * \param   self    The instance of `struct dir` to initialize.
 */
void
dir_tx_uninit(struct dir_tx* self);

/**
 * Prepare after acquiring the first reference on the transaction-local
 * directory state.
 */
void
dir_tx_prepare(struct dir_tx* self, struct dir* dir,
               struct picotm_error* error);

/**
 * Clean up after releasing final reference.
 */
void
dir_tx_release(struct dir_tx* self);

void
dir_tx_try_rdlock_field(struct dir_tx* self, enum dir_field field,
                        struct picotm_error* error);

void
dir_tx_try_wrlock_field(struct dir_tx* self, enum dir_field field,
                        struct picotm_error* error);

void
dir_tx_finish(struct dir_tx* self);
