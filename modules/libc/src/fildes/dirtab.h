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
#include "dir.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct fildes_dirtab {
    struct dir       tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_DIRTAB_INITIALIZER           \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

void
fildes_dirtab_init(struct fildes_dirtab* self, struct picotm_error* error);

void
fildes_dirtab_uninit(struct fildes_dirtab* self);

/**
 * Returns a reference to a dir structure for the given file descriptor.
 * \param       self    The dir table.
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns A referenced instance of `struct dir` that refers to the file
 *          descriptor's directory.
 */
struct dir*
fildes_dirtab_ref_fildes(struct fildes_dirtab* self, int fildes,
                         struct picotm_error* error);

/**
 * Returns the index of a dir structure within the dir table.
 * \param   self    The dir table.
 * \param   dir     An dir structure.
 * \returns The dir structure's index in the dir table.
 */
size_t
fildes_dirtab_index(struct fildes_dirtab* self, struct dir* dir);
