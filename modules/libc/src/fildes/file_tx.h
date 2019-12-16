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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-ref.h"
#include "picotm/picotm-lib-slist.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
struct file;

/**
 * Holds transaction-local state for a file.
 */
struct file_tx {

    struct picotm_ref16 ref;

    struct picotm_slist list_entry;

    struct file* file;

    const struct file_tx_ops* ops;
};

static inline struct file_tx*
file_tx_of_list_entry(struct picotm_slist* item)
{
    return picotm_containerof(item, struct file_tx, list_entry);
}

/**
 * \brief Init transaction-local file state.
 * \param   self    A file transaction.
 * \param   ops     The file type's constants and operations.
 */
void
file_tx_init(struct file_tx* self, const struct file_tx_ops* ops);

/**
 * \brief Uninit transaction-local file state.
 * \param   self    A file transaction.
 */
void
file_tx_uninit(struct file_tx* self);

/**
 * \brief Returns the file type.
 * \param   self    A file transaction.
 * \returns The open file description's file type.
 */
enum picotm_libc_file_type
file_tx_file_type(const struct file_tx* self);

/**
 * \brief Sets up a file transaction or acquires a reference on an
 *        already set-up instance.
 * \param       self    A file transaction.
 * \param       ofd     The global file state.
 * \param       data    User-data.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_ref_or_set_up(struct file_tx* self, struct file* file,
                      void* data, struct picotm_error* error);

/**
 * Acquire a reference on a file transaction.
 * \param       self    A file transaction.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_ref(struct file_tx* self, struct picotm_error* error);

/**
 * Release a reference on a file transaction.
 * \param   self    A file transaction.
 */
void
file_tx_unref(struct file_tx* self);

/**
 * Returns true if the transactions holds a reference to the
 * transaction-local file state.
 */
bool
file_tx_holds_ref(struct file_tx* self);

/*
 * Module interfaces
 */

/**
 * \brief Finishes the transaction-local file state at the end of a
 *        transaction.
 * \param   self    The transaction-local file state.
 */
void
file_tx_finish(struct file_tx* self);
