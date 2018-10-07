/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
