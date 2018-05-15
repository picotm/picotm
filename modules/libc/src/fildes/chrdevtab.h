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
#include "chrdev.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct chrdev;
struct picotm_error;

struct fildes_chrdevtab {
    struct chrdev    tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_CHRDEVTAB_INITIALIZER        \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

void
fildes_chrdevtab_uninit(struct fildes_chrdevtab* self);

/**
 * Returns a reference to an chrdev structure for the given file descriptor.
 * \param       self    The chrdev table.
 * \param       fildes  A file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns A referenced instance of `struct chrdev` that refers to the file
 *          descriptor's character device.
 */
struct chrdev*
fildes_chrdevtab_ref_fildes(struct fildes_chrdevtab* self, int fildes,
                            struct picotm_error* error);

/**
 * Returns the index of an chrdev structure within the chrdev table.
 * \param   self    The chrdev table.
 * \param   chrdev  An chrdev structure.
 * \returns The chrdev structure's index in the chrdev table.
 */
size_t
fildes_chrdevtab_index(struct fildes_chrdevtab* self, struct chrdev* chrdev);
