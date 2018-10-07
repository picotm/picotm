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

#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include <stdbool.h>
#include <sys/types.h>
#include "ofd_id.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_rwstate;

/**
 * Enumerates fields of `struct ofd`.
 */
enum ofd_field {
    OFD_FIELD_FILE_OFFSET,
    OFD_FIELD_STATE,
    NUMBER_OF_OFD_FIELDS
};

/**
 * Represents an open file description.
 */
struct ofd {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The open file description's unique id. */
    struct ofd_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_OFD_FIELDS];
};

/**
 * \brief Initializes an open file description.
 * \param       self    The open file description to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_init(struct ofd* self, struct picotm_error* error);

/**
 * \brief Uninitializes an open file description.
 * \param   self    The open file description to uninitialize.
 */
void
ofd_uninit(struct ofd* self);

/**
 * \brief Acquires a reference on an open file description.
 * \param       self    The open file description.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_ref(struct ofd* self, struct picotm_error* error);

/**
 * \brief Sets up an open file description or acquires a reference
 *        on an already set-up instance.
 * \param       self    The open file description.
 * \param       fildes  A file descriptor for the setup.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_ref_or_set_up(struct ofd* self, int fildes,
                  struct picotm_error* error);

/**
 * \brief Releases a reference on an open file description.
 * \param   self    The open file description.
 */
void
ofd_unref(struct ofd* self);

/**
 * \brief Compares the open file description's id to a reference id and
 *        acquires a reference if both id's are equal. The instance is
 *        set up from the provided file descriptor if necessary.
 * \param       self        The open file description.
 * \param       id          The id to compare to.
 * \param       fildes      A file descriptor for the setup.
 * \param       ne_fildes   True to request non-equal file descriptors, false
 *                          otherwise.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
ofd_cmp_and_ref_or_set_up(struct ofd* self, const struct ofd_id* id,
                          int fildes, bool ne_fildes,
                          struct picotm_error* error);

/**
 * \brief Compares the open file description's id to a reference id and
 *        acquires a reference if both id's are equal.
 * \param       self        The open file description.
 * \param       id          The id to compare to.
 * \param       ne_fildes   True to request non-equal file descriptors, false
 *                          otherwise.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
ofd_cmp_and_ref(struct ofd* self, const struct ofd_id* id, bool ne_fildes,
                struct picotm_error* error);

/**
 * \brief Tries to acquire a reader lock on an open file description.
 * \param       self        The open file description.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_try_rdlock_field(struct ofd* self, enum ofd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on an open file description.
 * \param       self        The open file description.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
ofd_try_wrlock_field(struct ofd* self, enum ofd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Releases a lock on an open file description.
 * \param   self    The open file description.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
ofd_unlock_field(struct ofd* self, enum ofd_field field,
                 struct picotm_rwstate* rwstate);
