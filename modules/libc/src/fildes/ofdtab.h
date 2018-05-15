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
#include <stdbool.h>
#include <stddef.h>
#include "ofd.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct fildes_ofdtab {
    struct ofd       tab[MAXNUMFD];
    size_t           len;
    pthread_rwlock_t rwlock;
};

#define FILDES_OFDTAB_INITIALIZER           \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER    \
}

void
fildes_ofdtab_init(struct fildes_ofdtab* self, struct picotm_error* error);

void
fildes_ofdtab_uninit(struct fildes_ofdtab* self);

/**
 * Returns a reference to an ofd structure for the given file descriptor.
 * \param       self            The ofd table.
 * \param       fildes          A file descriptor.
 * \param       newly_created   True if the open file description has been
 *                              newly created.
 * \param[out]  error           Returns an error.
 * \returns A referenced instance of `struct ofd` that refers to the file
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
struct ofd*
fildes_ofdtab_ref_fildes(struct fildes_ofdtab* self, int fildes,
                         bool newly_created, struct picotm_error* error);

/**
 * Returns the index of an ofd structure within the ofd table.
 * \param   self    The ofd table.
 * \param   ofd     An ofd structure.
 * \returns The ofd structure's index in the ofd table.
 */
size_t
fildes_ofdtab_index(struct fildes_ofdtab* self, struct ofd* ofd);
