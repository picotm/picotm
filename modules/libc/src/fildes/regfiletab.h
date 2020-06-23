/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 * Copyright (c) 2020       Thomas Zimmermann <contact@tzimmermann.org>
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

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include "regfile.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct fildes_regfiletab {
    struct regfile   tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_REGFILETAB_INITIALIZER       \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

void
fildes_regfiletab_init(struct fildes_regfiletab* self,
                       struct picotm_error* error);

void
fildes_regfiletab_uninit(struct fildes_regfiletab* self);

/**
 * Returns a reference to an regfile structure for the given file descriptor.
 * \param       self        The regfile table.
 * \param       fildes      A file descriptor.
 * \param       new_file    True if the open file description has been
 *                          newly created.
 * \param[out]  error       Returns an error.
 * \returns A referenced instance of `struct regfile` that refers to the file
 *          descriptor's open file description.
 *
 * We cannot distiguish between open file descriptions. Two
 * file descriptors refering to the same buffer might share
 * the same open file description, or not.
 *
 * As a workaround, we only allow one file descriptor per file
 * buffer at the same time. If we see a second file descriptor
 * refering to a buffer that is already in use, the look-up
 * fails.
 *
 * For a solution, Linux (or any other Unix) has to provide a
 * unique id for each open file description, or at least give
 * us a way of figuring out the relationship between file descriptors
 * and open file descriptions.
 */
struct regfile*
fildes_regfiletab_ref_fildes(struct fildes_regfiletab* self, int fildes,
                             bool new_file, struct picotm_error* error);

/**
 * Returns the index of an regfile structure within the regfile table.
 * \param   self    The regfile table.
 * \param   regfile     An regfile structure.
 * \returns The regfile structure's index in the regfile table.
 */
size_t
fildes_regfiletab_index(struct fildes_regfiletab* self,
                        struct regfile* regfile);
