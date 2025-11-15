/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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
