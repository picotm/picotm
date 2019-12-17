/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019   Thomas Zimmermann <contact@tzimmermann.org>
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
#include "picotm/picotm-lib-rwlock.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include "filebuf_id.h"

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
 * Enumerates fields of `struct pipebuf`.
 */
enum pipebuf_field {
    PIPEBUF_FIELD_READ_END,
    PIPEBUF_FIELD_STATE,
    PIPEBUF_FIELD_WRITE_END,
    NUMBER_OF_PIPEBUF_FIELDS
};

/**
 * Represents a pipebuf's open file description.
 */
struct pipebuf {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The pipe buffer's unique id. */
    struct filebuf_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_PIPEBUF_FIELDS];
};

/**
 * \brief Initializes a pipebuf instance.
 * \param       self    The pipebuf instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
pipebuf_init(struct pipebuf* self, struct picotm_error* error);

/**
 * \brief Uninitializes a pipebuf instance.
 * \param   self    The pipebuf instance to uninitialize.
 */
void
pipebuf_uninit(struct pipebuf* self);

/**
 * \brief Sets up an instance of `struct pipebuf` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The pipebuf instance.
 * \param       fildes  The pipebuf's pipebuf descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
pipebuf_ref_or_set_up(struct pipebuf* self, int fildes,
                      struct picotm_error* error);

/**
 * \brief Compares the pipebuf's id to an id and acquires a reference if both
 *        id's are equal. The pipebuf instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The pipebuf instance.
 * \param       fildes      The pipebuf's pipebuf descriptor.
 * \param       id          The id to compare to.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
pipebuf_ref_or_set_up_if_id(struct pipebuf* self, int fildes,
                            const struct filebuf_id* id,
                            struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct pipebuf`.
 * \param       self    The pipebuf instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
pipebuf_ref(struct pipebuf* self, struct picotm_error* error);

/**
 * \brief Compares the pipebuf's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The pipebuf instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the pipebuf's id
 *          is less than, equal to, or greater than the given id.
 */
int
pipebuf_ref_if_id(struct pipebuf* self, const struct filebuf_id* id);

/**
 * \brief Unreferences a pipebuf.
 * \param   self    The pipebuf instance.
 */
void
pipebuf_unref(struct pipebuf* self);

/**
 * \brief Tries to acquire a reader lock on a pipebuf.
 * \param       self        The pipebuf instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
pipebuf_try_rdlock_field(struct pipebuf* self, enum pipebuf_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a pipebuf.
 * \param       self        The pipebuf instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
pipebuf_try_wrlock_field(struct pipebuf* self, enum pipebuf_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Releases a lock on a pipebuf.
 * \param   self    The pipebuf instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
pipebuf_unlock_field(struct pipebuf* self, enum pipebuf_field field,
                     struct picotm_rwstate* rwstate);
