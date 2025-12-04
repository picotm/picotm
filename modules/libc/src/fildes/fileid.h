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
#include <sys/types.h>

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
 * By default, an open file does not have an id by itself. Picotm
 * constructs the id from the id of the file's file buffer and the
 * file descriptor that refers to the file.
 *
 * If two file descriptors refer to the same file, they create two
 * different ids. Even though the file may be the same, these ids
 * must compare as *different.* It's a shortcoming resulting from the
 * non-existence of unique ids for open files.
 */
struct file_id {
    dev_t  dev;
    ino_t  ino;
    int fildes;
};

/**
 * Initializes a file id with values
 * \param[out]  self    The file id to initialize.
 * \param       dev     The device number.
 * \param       ino     The inode number.
 */
void
file_id_init(struct file_id* self, dev_t dev, ino_t ino);

/**
 * \brief Initializes a file id from file descriptor.
 * \param[out]  self    The file id to initialize.
 * \param       fildes  The file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_id_init_from_fildes(struct file_id* self, int fildes,
                            struct picotm_error* error);

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
 * \returns True if the file ids are equal, or false otherwise.
 *
 * Compares file ids. Also works for file ids that have been cleared, which
 * always compare as equal.
 */
bool
file_id_cmp_eq(const struct file_id* lhs, const struct file_id* rhs, struct picotm_error* error);
