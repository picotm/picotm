/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019-2020  Thomas Zimmermann <contact@tzimmermann.org>
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
 * Enumerates fields of `struct sockbuf`.
 */
enum sockbuf_field {
    SOCKBUF_FIELD_RECV_END,
    SOCKBUF_FIELD_SEND_END,
    SOCKBUF_FIELD_STATE,
    NUMBER_OF_SOCKBUF_FIELDS
};

/**
 * Represents a sockbuf's open file description.
 */
struct sockbuf {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The socket buffer's unique id. */
    struct filebuf_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_SOCKBUF_FIELDS];
};

/**
 * \brief Initializes a sockbuf instance.
 * \param       self    The sockbuf instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
sockbuf_init(struct sockbuf* self, struct picotm_error* error);

/**
 * \brief Uninitializes a sockbuf instance.
 * \param   self    The sockbuf instance to uninitialize.
 */
void
sockbuf_uninit(struct sockbuf* self);

/**
 * \brief Sets up an instance of `struct sockbuf` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The sockbuf instance.
 * \param       fildes  The sockbuf's sockbuf descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
sockbuf_ref_or_set_up(struct sockbuf* self, int fildes,
                      struct picotm_error* error);

/**
 * \brief Compares the sockbuf's id to an id and acquires a reference if both
 *        id's are equal. The sockbuf instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The sockbuf instance.
 * \param       fildes      The sockbuf's sockbuf descriptor.
 * \param       id          The id to compare to.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the sockbuf's
 *          id is less than, equal to, or greater than the given id.
 */
int
sockbuf_ref_or_set_up_if_id(struct sockbuf* self, int fildes,
                            const struct filebuf_id* id,
                            struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct sockbuf`.
 * \param       self    The sockbuf instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
sockbuf_ref(struct sockbuf* self, struct picotm_error* error);

/**
 * \brief Compares the sockbuf's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The sockbuf instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the sockbuf's id
 *          is less than, equal to, or greater than the given id.
 */
int
sockbuf_ref_if_id(struct sockbuf* self, const struct filebuf_id* id);

/**
 * \brief Unreferences a sockbuf.
 * \param   self    The sockbuf instance.
 */
void
sockbuf_unref(struct sockbuf* self);

/**
 * \brief Tries to acquire a reader lock on a sockbuf.
 * \param       self        The sockbuf instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
sockbuf_try_rdlock_field(struct sockbuf* self, enum sockbuf_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a sockbuf.
 * \param       self        The sockbuf instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
sockbuf_try_wrlock_field(struct sockbuf* self, enum sockbuf_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Releases a lock on a sockbuf.
 * \param   self    The sockbuf instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
sockbuf_unlock_field(struct sockbuf* self, enum sockbuf_field field,
                     struct picotm_rwstate* rwstate);
