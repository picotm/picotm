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
 * Enumerates fields of `struct chrdev`.
 */
enum chrdev_field {
    CHRDEV_FIELD_FILE_MODE,
    CHRDEV_FIELD_FILE_OFFSET,
    CHRDEV_FIELD_STATE,
    NUMBER_OF_CHRDEV_FIELDS
};

/**
 * Represents a character device's open file description.
 */
struct chrdev {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The character device's unique id. */
    struct file_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock rwlock[NUMBER_OF_CHRDEV_FIELDS];
};

/**
 * \brief Initializes a chrdev instance.
 * \param       self    The chrdev instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
chrdev_init(struct chrdev* self, struct picotm_error* error);

/**
 * \brief Uninitializes a chrdev structure.
 * \param   self    The chrdev instance to uninitialize.
 */
void
chrdev_uninit(struct chrdev* self);

/**
 * \brief Sets up an instance of `struct chrdev` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The chrdev instance.
 * \param       fildes  The character device's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
chrdev_ref_or_set_up(struct chrdev* self, int fildes,
                     struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct chrdev`.
 * \param       self    The chrdev instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
chrdev_ref(struct chrdev* self, struct picotm_error* error);

/**
 * \brief Compares the chrdev's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The chrdev instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the chrdev's id
 *          is less than, equal to, or greater than the given id.
 */
int
chrdev_cmp_and_ref(struct chrdev* self, const struct file_id* id);

/**
 * \brief Compares the chrdev's id to an id and acquires a reference if both
 *        id's are equal. The chrdev instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The chrdev instance.
 * \param       id          The id to compare to.
 * \param       fildes      The character device's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
chrdev_cmp_and_ref_or_set_up(struct chrdev* self, const struct file_id* id,
                             int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a character device.
 * \param   self    The chrdev instance.
 */
void
chrdev_unref(struct chrdev* self);

/**
 * \brief Tries to acquire a reader lock on a character device.
 * \param       self        The chrdev instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
chrdev_try_rdlock_field(struct chrdev* self, enum chrdev_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a character device.
 * \param       self        The chrdev instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
chrdev_try_wrlock_field(struct chrdev* self, enum chrdev_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * \brief Releases a lock on a character device.
 * \param   self    The chrdev instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
chrdev_unlock_field(struct chrdev* self, enum chrdev_field field,
                    struct picotm_rwstate* rwstate);
