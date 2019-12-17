/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019   Thomas Zimmermann <contact@tzimmermann.org>
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
#include "pipebuf.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct fildes_pipebuftab {
    struct pipebuf tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_SEEKBUFTAB_INITIALIZER       \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

void
fildes_pipebuftab_init(struct fildes_pipebuftab* self,
                       struct picotm_error* error);

void
fildes_pipebuftab_uninit(struct fildes_pipebuftab* self);

/**
 * Returns a reference to a pipebuf structure for the given file descriptor.
 * \param       self    The pipebuf table.
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns A referenced instance of `struct pipebuf` that refers to the file
 *          descriptor's regular file.
 */
struct pipebuf*
fildes_pipebuftab_ref_fildes(struct fildes_pipebuftab* self, int fildes,
                             struct picotm_error* error);

/**
 * Returns the index of an pipebuf structure within the pipebuf table.
 * \param   self    The pipebuf table.
 * \param   pipebuf An pipebuf structure.
 * \returns The pipebuf structure's index in the pipebuf table.
 */
size_t
fildes_pipebuftab_index(struct fildes_pipebuftab* self,
                        struct pipebuf* pipebuf);
