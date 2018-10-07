/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "fileid.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * \brief The unique id of an open file description.
 *
 * By default, an open file description does not have an id by itself.
 * In picotm, we construct the id from the file id of the open file
 * description's file and a file descriptor that refers to the open
 * file description.
 *
 * If two file descriptors refer to the same open file description,
 * they create two different ids. Even though the open file description
 * is the same, these ids must compare as *different*. The look-up code
 * fixes this ad-hoc. It's a short coming resulting fom the non-existance
 * of unique system ids for open file descriptions.
 */
struct ofd_id {
    int fildes;
    struct file_id file_id;
};

/**
 * \brief Initializes an open-file-description id.
 */
void
ofd_id_init(struct ofd_id* self);

/**
 * \brief Initializes an open-file-description id from a file descriptor.
 * \param   self        An open-file-description id.
 * \param   fildes      The open file description's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_id_init_from_fildes(struct ofd_id* self, int fildes,
                        struct picotm_error* error);

/**
 * \brief Uninitializes an open-file-description id.
 * \param self  An open-file-descriptinon id.
 */
void
ofd_id_uninit(struct ofd_id* self);

/**
 * \brief Sets an open-file-description id from a file descriptor.
 * \param       self    An open-file-description id.
 * \param       fildes  The open file description's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
ofd_id_set_from_fildes(struct ofd_id* self, int fildes,
                       struct picotm_error* error);

/**
 * \brief Clears an open-file-description id to an empty state.
 * \param   self    An open-file-description id.
 */
void
ofd_id_clear(struct ofd_id* self);

/**
 * \brief Compares two open-file-description ids.
 * \param   lhs An open-file-description id.
 * \param   rhs An open-file-description id.
 * \returns A value less than, equal to, or greater than 0 if lhs is
 *          less than, equal to, or greater than rhs.
 */
int
ofd_id_cmp(const struct ofd_id* lhs, const struct ofd_id* rhs);

/**
 * \brief Compares two open-file-description ids with different file
 *        descriptors.
 * \param       lhs     An open-file-description id.
 * \param       rhs     An open-file-description id.
 * \param[out]  error   Returns an error to the caller.
 * \returns A value less than, equal to, or greater than 0 if the file id
 *          of lhs is less than, equal to, or greater than the file id of
 *          rhs.
 */
int
ofd_id_cmp_ne_fildes(const struct ofd_id* lhs, const struct ofd_id* rhs,
                     struct picotm_error* error);
