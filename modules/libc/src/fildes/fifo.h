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
#include "fileid.h"

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
 * Enumerates fields of `struct fifo`.
 */
enum fifo_field {
    FIFO_FIELD_FILE_MODE,
    FIFO_FIELD_READ_END,
    FIFO_FIELD_STATE,
    FIFO_FIELD_WRITE_END,
    NUMBER_OF_FIFO_FIELDS
};

/**
 * Represents a FIFO's open file description.
 */
struct fifo {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The FIFO's unique id. */
    struct file_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_FIFO_FIELDS];
};

/**
 * \brief Initializes a FIFO instance.
 * \param       self    The FIFO instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
fifo_init(struct fifo* self, struct picotm_error* error);

/**
 * \brief Uninitializes a FIFO instance.
 * \param   self    The FIFO instance to uninitialize.
 */
void
fifo_uninit(struct fifo* self);

/**
 * \brief Sets up an instance of `struct fifo` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The FIFO instance.
 * \param       fildes  The FIFO's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
fifo_ref_or_set_up(struct fifo* self, int fildes, struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct fifo`.
 * \param       self    The FIFO instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
fifo_ref(struct fifo* self, struct picotm_error* error);

/**
 * \brief Compares the FIFO's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The FIFO instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the FIFO's id
 *          is less than, equal to, or greater than the given id.
 */
int
fifo_cmp_and_ref(struct fifo* self, const struct file_id* id);

/**
 * \brief Compares the FIFO's id to an id and acquires a reference if both
 *        id's are equal. The FIFO instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The FIFO instance.
 * \param       id          The id to compare to.
 * \param       fildes      The FIFO's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
fifo_cmp_and_ref_or_set_up(struct fifo* self, const struct file_id* id,
                           int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a FIFO.
 * \param   self    The FIFO instance.
 */
void
fifo_unref(struct fifo* self);

/**
 * \brief Tries to acquire a reader lock on a FIFO.
 * \param       self        The FIFO instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
fifo_try_rdlock_field(struct fifo* self, enum fifo_field field,
                      struct picotm_rwstate* rwstate,
                      struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a FIFO.
 * \param       self        The FIFO instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
fifo_try_wrlock_field(struct fifo* self, enum fifo_field field,
                      struct picotm_rwstate* rwstate,
                      struct picotm_error* error);

/**
 * \brief Releases a lock on a character device.
 * \param   self    The FIFO instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
fifo_unlock_field(struct fifo* self, enum fifo_field field,
                  struct picotm_rwstate* rwstate);
