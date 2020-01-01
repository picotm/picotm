/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
 * Copyright (c) 2020   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <stdbool.h>
#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * A unique id for a file buffer.
 */
struct filebuf_id {
    dev_t  dev;
    ino_t  ino;
};

/**
 * Initializes a file-buffer id with values
 * \param[out]  self    The file-buffer id to initialize.
 * \param       dev     The device number.
 * \param       ino     The inode number.
 */
void
filebuf_id_init(struct filebuf_id* self, dev_t dev, ino_t ino);

/**
 * \brief Initializes a file-buffer id from file descriptor.
 * \param[out]  self    The file-buffer id to initialize.
 * \param       fildes  The file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
filebuf_id_init_from_fildes(struct filebuf_id* self, int fildes,
                            struct picotm_error* error);

/**
 * \brief Clears a file-buffer id.
 * \param   self    The file-buffer id to clear.
 */
void
filebuf_id_clear(struct filebuf_id* self);

/**
 * \brief Tests file-buffer id for emptiness.
 * \param   self    The file-buffer id.
 * \returns True if the file is has not been initialized, or false otherwise.
 */
bool
filebuf_id_is_empty(const struct filebuf_id* self);

/**
 * \return Compares two file-buffer ids, returns value as for strcmp.
 * \param   lhs The left-hand-side file-buffer id.
 * \param   rhs The right-hand-side file-buffer id.
 * \returns A value less than, equal to or greater than zero of the value
 *          of lhs is less than, equal to or greater than the value of
 *          rhws.
 */
int
filebuf_id_cmp(const struct filebuf_id* lhs, const struct filebuf_id* rhs);
