/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

/**
 * \cond impl || libc_impl || libc_impl_cwd
 * \ingroup libc_impl
 * \ingroup libc_impl_cwd
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_rwstate;

/**
 * Enumerates the fields of `struct cwd`.
 */
enum cwd_field {
    CWD_FIELD_STRING,
    NUMBER_OF_CWD_FIELDS
};

/**
 * Represents the global state of the current working directory.
 */
struct cwd {
    struct picotm_rwlock    rwlock[NUMBER_OF_CWD_FIELDS];
};

/**
 * Initialized a CWD structure
 *
 * \param   self    The CWD structure.
 */
void
cwd_init(struct cwd* self);

/**
 * Uninitialized a CWD structure
 *
 * \param   self    The CWD structure.
 */
void
cwd_uninit(struct cwd* self);

/**
 * Tries to acquire a reader lock on the current working directory.
 *
 * \param       self    The CWD structure.
 * \param       field   The reader lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
cwd_try_rdlock_field(struct cwd* self, enum cwd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * Tries to acquire a writer lock on the current working directory.
 *
 * \param       self    The CWD structure.
 * \param       field   The writer lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
cwd_try_wrlock_field(struct cwd* self, enum cwd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * Releases a reader/writer lock on the current working directory.
 *
 * \param   self    The CWD structure.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader-writer state.
 */
void
cwd_unlock_field(struct cwd* self, enum cwd_field field,
                 struct picotm_rwstate* rwstate);
