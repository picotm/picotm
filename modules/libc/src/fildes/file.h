/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
 * Copyright (c) 2020   Thomas Zimmermann
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
#include "picotm/picotm-lib-ref.h"
#include "picotm/picotm-lib-shared-ref-obj.h"
#include "fileid.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

/**
 * Represents a file.
 */
struct file {
    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    /** The file's unique id. */
    struct file_id id;
};

/**
 * \brief Initializes a file instance.
 * \param       self    The file instance to initialize.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_init(struct file* self, struct picotm_error* error);

/**
 * \brief Uninitializes a file instance.
 * \param   self    The file instance to uninitialize.
 */
void
file_uninit(struct file* self);

/**
 * \brief Sets up an instance of `struct file` or acquires a reference
 *        on an already set-up instance.
 * \param       self    The file instance.
 * \param       fildes  The file's file descriptor.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_ref_or_set_up(struct file* self, int fildes, struct picotm_error* error);

/**
 * \brief Compares the file's id to an id and acquires a reference if both
 *        id's are equal. The file instance is set up from the provided
 *        file descriptor if necessary.
 * \param       self        The file instance.
 * \param       fildes      The file's file descriptor.
 * \param       id          The id to compare to.
 * \param       new_file    True if the file was create by the transaction;
 *                          false otherwise.
 * \param[out]  error       Returns an error ot the caller.
 * \returns A value less than, equal to, or greater than if the file's id is
 *          less than, equal to, or greater than the given id.
 */
int
file_ref_or_set_up_if_id(struct file* self, int fildes, bool new_file,
                         const struct file_id* id,
                         struct picotm_error* error);

/**
 * \brief Acquires a reference on an instance of `struct file`.
 * \param       self    The file instance.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_ref(struct file* self, struct picotm_error* error);

/**
 * \brief Compares the file's id to an id and acquires a reference if both
 *        id's are equal.
 * \param   self        The file instance.
 * \param   id          The id to compare to.
 * \param   new_file    True if the file was create by the transaction;
 *                      false otherwise.
 * \returns A value less than, equal to, or greater than if the file's id
 *          is less than, equal to, or greater than the given id.
 */
int
file_ref_if_id(struct file* self, const struct file_id* id, bool new_file,
               struct picotm_error* error);

/**
 * \brief Unreferences a file.
 * \param   self    The file instance.
 */
void
file_unref(struct file* self);
