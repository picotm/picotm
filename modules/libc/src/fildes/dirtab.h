/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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
