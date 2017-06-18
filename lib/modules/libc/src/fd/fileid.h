/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
 * A unique id for a file.
 */
struct file_id {
    dev_t  dev;
    ino_t  ino;
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
 * \return Compares two file ids, returns value as for strcmp.
 * \param   lhs The left-hand-side file id.
 * \param   rhs The right-hand-side file id.
 * \returns A value less than, equal to or greater than zero of the value
 *          of lhs is less than, equal to or greater than the value of
 *          rhws.
 */
int
file_id_cmp(const struct file_id* lhs, const struct file_id* rhs);
