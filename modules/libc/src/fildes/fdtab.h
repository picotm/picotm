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

#include "picotm/picotm-lib-rwlock.h"
#include <stddef.h>
#include <pthread.h>
#include "fd.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct fd;
struct picotm_error;
struct picotm_rwstate;

struct fildes_fdtab {
    struct fd                tab[MAXNUMFD];
    size_t                   len;
    pthread_rwlock_t         rwlock;
    struct picotm_rwlock     lock;
};

#define FILDES_FDTAB_INITIALIZER            \
{                                           \
    .len = 0,                               \
    .rwlock = PTHREAD_RWLOCK_INITIALIZER,   \
    .lock = PICOTM_RWLOCK_INITIALIZER       \
}

void
fildes_fdtab_init(struct fildes_fdtab* self, struct picotm_error* error);

void
fildes_fdtab_uninit(struct fildes_fdtab* self);

struct fd*
fildes_fdtab_ref_fildes(struct fildes_fdtab* self, int fildes,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error);

struct fd*
fildes_fdtab_get_fd(struct fildes_fdtab* self, int fildes);

void
fildes_fdtab_try_rdlock(struct fildes_fdtab* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error);

void
fildes_fdtab_try_wrlock(struct fildes_fdtab* self,
                        struct picotm_rwstate* lock_state,
                        struct picotm_error* error);

void
fildes_fdtab_unlock(struct fildes_fdtab* self,
                    struct picotm_rwstate* lock_state);
