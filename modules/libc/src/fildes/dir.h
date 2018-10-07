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
 * Enumerates fields of `struct dir`.
 */
enum dir_field {
    DIR_FIELD_FILE_MODE,
    DIR_FIELD_STATE,
    NUMBER_OF_DIR_FIELDS
};

/**
 * Represents a directory's open file description.
 */
struct dir {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The directory's unique id. */
    struct file_id id;

    /** Reader/writer state locks. */
    struct picotm_rwlock rwlock[NUMBER_OF_DIR_FIELDS];
};

/**
 * \brief Initializes a dir instance.
 * \param       self    The dir instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
dir_init(struct dir* self, struct picotm_error* error);

/**
 * \brief Uninitializes a dir structure.
 * \param   self    The dir instance to uninitialize.
 */
void
dir_uninit(struct dir* self);

/**
 * \brief Sets up an instance of `struct dir` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The dir instance.
 * \param       fildes  The directory's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
dir_ref_or_set_up(struct dir* self, int fildes, struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct dir`.
 * \param       self    The dir instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
dir_ref(struct dir* self, struct picotm_error* error);

/**
 * \brief Compares the dir's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self    The dir instance.
 * \param   id      The id to compare to.
 * \returns A value less than, equal to, or greater than if the dir's id
 *          is less than, equal to, or greater than the given id.
 */
int
dir_cmp_and_ref(struct dir* self, const struct file_id* id);

/**
 * \brief Compares the dir's id to an id and acquires a reference if both
 *        id's are equal. The dir instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The dir instance.
 * \param       id          The id to compare to.
 * \param       fildes      The directory's file descriptor.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the ofd's id is
 *          less than, equal to, or greater than the given id.
 */
int
dir_cmp_and_ref_or_set_up(struct dir* self, const struct file_id* id,
                          int fildes, struct picotm_error* error);

/**
 * \brief Unreferences a directory.
 * \param   self    The dir instance.
 */
void
dir_unref(struct dir* self);

/**
 * \brief Tries to acquire a reader lock on a directory.
 * \param       self        The dir instance.
 * \param       field       The reader/writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
dir_try_rdlock_field(struct dir* self, enum dir_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a directory.
 * \param       self        The dir instance.
 * \param       field       The reader/writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
dir_try_wrlock_field(struct dir* self, enum dir_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * \brief Releases a lock on a directory.
 * \param   self    The dir instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
dir_unlock_field(struct dir* self, enum dir_field field,
                 struct picotm_rwstate* rwstate);
