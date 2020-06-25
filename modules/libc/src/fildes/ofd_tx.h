/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 * Copyright (c) 2020       Thomas Zimmermann <contact@tzimmermann.org>
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
#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-lib-slist.h"
#include "ofd.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct file_tx;

/**
 * Holds the transaction-local state for an open file description.
 */
struct ofd_tx {

    struct picotm_ref16 ref;

    struct picotm_slist list_entry;

    struct ofd* ofd;

    struct file_tx* file_tx;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_OFD_FIELDS];
};

static inline struct ofd_tx*
ofd_tx_of_list_entry(struct picotm_slist* item)
{
    return picotm_containerof(item, struct ofd_tx, list_entry);
}

/**
 * Init transaction-local open-file-description state.
 * \param   self    Transaction-local open file description.
 */
void
ofd_tx_init(struct ofd_tx* self);

/**
 * Uninit transaction-local open-file-description state.
 * \param   self    Transaction-local open file description.
 */
void
ofd_tx_uninit(struct ofd_tx* self);

/**
 * \brief Tests if a transaction-local open-file-description state holds a
 *        reference to a global state.
 * \param   self    An transaction-local open-file-description state structure.
 * \returns True if the transaction-local state references global state,
 *          false otherwise.
 */
bool
ofd_tx_holds_ref(struct ofd_tx* self);

/**
 * \brief Compare id of open file description to reference id.
 * \param   self    Transaction-local open file description.
 * \param   id      An open-file-description id.
 * \returns A value less than, equal to, or greater than 0 if the
 *          open file description's id is less than, equal to, or
 *          greater than the reference id.
 */
int
ofd_tx_cmp_with_id(const struct ofd_tx* self, const struct ofd_id* id);

/*
 * Reference counting
 */

/**
 * \brief Sets up a transaction-local open file description or acquires
 *        a reference on an already set-up instance.
 * \param       self    The transaction-local open file description.
 * \param       ofd     The global open file description.
 * \param       file_tx The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_tx_ref_or_set_up(struct ofd_tx* self, struct ofd* ofd,
                     struct file_tx* file_tx, struct picotm_error* error);

/**
 * \brief Acquires a reference on the transaction-local open file description.
 * \param       self    The transaction-local open file description.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_tx_ref(struct ofd_tx* self, struct picotm_error* error);

/**
 * \brief Releases a reference on the transaction-local open file description.
 * \param   self    The transaction-local open file description.
 */
void
ofd_tx_unref(struct ofd_tx* self);

/*
 * Locking
 */

/**
 * \brief Tries to acquire a reader lock on an open file description.
 * \param       self        The transaction-local open file description.
 * \param       field       The reader lock's field.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_tx_try_rdlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on an open file description.
 * \param       self        The transaction-local open file description.
 * \param       field       The reader lock's field.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_tx_try_wrlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error);

/*
 * Module interface
 */

/**
 * Cleans up the data structures for at the end of a transaction.
 */
void
ofd_tx_finish(struct ofd_tx* self);
