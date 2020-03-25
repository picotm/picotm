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
#include "file.h"

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

    /** Base object. */
    struct file base;

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
