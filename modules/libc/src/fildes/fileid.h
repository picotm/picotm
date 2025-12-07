/*
 * picotm - A system-level transaction manager
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

#include "picotm/picotm-error.h"
#include <stdbool.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

/**
 * A unique id for a file.
 *
 * An open file does not have an id by itself. Picotm constructs
 * the id by querying the operating system for equality of the file
 * descriptor's file.
 */
struct file_id {
    int fildes;
};

#define FILE_ID_INITIALIZER(__fildes)   \
    {                                   \
        .fildes = (__fildes),           \
    }

/**
 * \brief Initializes a file id from file descriptor.
 * \param[out]  self    The file id to initialize.
 * \param       fildes  The file descriptor.
 */
void
file_id_init(struct file_id* self, int fildes);

/**
 * \brief Clears a file id.
 * \param   self    The file id to clear.
 */
void
file_id_clear(struct file_id* self);

/**
 * \brief Tests file id for emptiness.
 * \param   self    The file id.
 * \returns True if the file is has not been initialized, or false otherwise.
 */
bool
file_id_is_empty(const struct file_id* self);

/**
 * \brief Compares two file ids for equality.
 * \param   lhs The left-hand-side file id.
 * \param   rhs The right-hand-side file id.
 * \param[out]  error   Returns an error to the caller.
 * \returns False if the file ids are not equal, or true otherwise.
 *
 * Compares file ids. Also works for file ids that have been cleared, which
 * always compare as equal.
 */
bool
file_id_cmp_eq(const struct file_id* lhs, const struct file_id* rhs, struct picotm_error* error);
