/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2019  Thomas Zimmermann <contact@tzimmermann.org>
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
struct rwcountermap;

/**
 * Enumerates fields of `struct regfile`.
 */
enum regfile_field {
    REGFILE_FIELD_FILE_MODE,
    REGFILE_FIELD_STATE,
    NUMBER_OF_REGFILE_FIELDS
};

/**
 * Represents a file's open file description.
 */
struct regfile {

    /** Base object. */
    struct file base;

    /** Reader/writer state locks. */
    struct picotm_rwlock  rwlock[NUMBER_OF_REGFILE_FIELDS];
};

static inline struct regfile*
regfile_of_base(struct file* base)
{
    return picotm_containerof(base, struct regfile, base);
}

/**
 * \brief Initializes a file instance.
 * \param       self    The file instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
regfile_init(struct regfile* self, struct picotm_error* error);

/**
 * \brief Uninitializes a file instance.
 * \param   self    The file instance to uninitialize.
 */
void
regfile_uninit(struct regfile* self);

/**
 * \brief Tries to acquire a reader lock on a file.
 * \param       self        The file instance.
 * \param       field       The reader lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
regfile_try_rdlock_field(struct regfile* self, enum regfile_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Tries to acquire a writer lock on a file.
 * \param       self        The file instance.
 * \param       field       The writer lock's field.
 * \param       rwstate     The transaction's reader/writer state.
 * \param[out]  error       Returns an error ot the caller.
 */
void
regfile_try_wrlock_field(struct regfile* self, enum regfile_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error);

/**
 * \brief Releases a lock on a file.
 * \param   self    The file instance.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader/writer state.
 */
void
regfile_unlock_field(struct regfile* self, enum regfile_field field,
                     struct picotm_rwstate* rwstate);
