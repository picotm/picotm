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

#include <pthread.h>
#include <stddef.h>
#include "fifo.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct fildes_fifotab {
    struct fifo      tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_FIFOTAB_INITIALIZER          \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

void
fildes_fifotab_init(struct fildes_fifotab* self, struct picotm_error* error);

void
fildes_fifotab_uninit(struct fildes_fifotab* self);

/**
 * Returns a reference to a fifo structure for the given file descriptor.
 * \param       self    The fifo table.
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an errorto the caller.
 * \returns A referenced instance of `struct fifo` that refers to the file
 *          descriptor's FIFO buffer.
 */
struct fifo*
fildes_fifotab_ref_fildes(struct fildes_fifotab* self, int fildes,
                          struct picotm_error* error);

/**
 * Returns the index of an fifo structure within the fifo table.
 * \param   self    The fifo table.
 * \param   fifo    An fifo structure.
 * \returns The fifo structure's index in the fifo table.
 */
size_t
fildes_fifotab_index(struct fildes_fifotab* self, struct fifo* fifo);
