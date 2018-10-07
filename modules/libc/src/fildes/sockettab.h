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
#include "socket.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct fildes_sockettab {
    struct socket    tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_SOCKETTAB_INITIALIZER        \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

void
fildes_sockettab_init(struct fildes_sockettab* self,
                      struct picotm_error* error);

void
fildes_sockettab_uninit(struct fildes_sockettab* self);

/**
 * Returns a reference to a socket structure for the given file descriptor.
 * \param       self    The socket table.
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to caller.
 * \returns A referenced instance of `struct socket` that refers to the file
 *          descriptor's socket.
 */
struct socket*
fildes_sockettab_ref_fildes(struct fildes_sockettab* self, int fildes,
                            struct picotm_error* error);

/**
 * Returns the index of an socket structure within the socket table.
 * \param   self    The socket table.
 * \param   socket  An socket structure.
 * \returns The socket structure's index in the socket table.
 */
size_t
fildes_sockettab_index(struct fildes_sockettab* self, struct socket* socket);
