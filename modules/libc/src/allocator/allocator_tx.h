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

#include <stddef.h>
#include "allocator_event.h"

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \ingroup libc_impl
 * \ingroup libc_impl_allocator
 * \file
 * \endcond
 */

struct allocator_log;
struct picotm_error;

/**
 * A transaction on the memory allocation.
 */
struct allocator_tx {
    struct allocator_log* log;
};

void
allocator_tx_init(struct allocator_tx* self, struct allocator_log* log);

void
allocator_tx_uninit(struct allocator_tx* self);

void
allocator_tx_exec_free(struct allocator_tx* self, void* ptr,
                       struct picotm_error* error);

void*
allocator_tx_exec_malloc(struct allocator_tx* self, size_t size,
                         struct picotm_error* error);

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
int
allocator_tx_exec_posix_memalign(struct allocator_tx* self, void** memptr,
                                 size_t alignment, size_t size,
                                 struct picotm_error* error);
#endif

void
allocator_tx_apply_event(struct allocator_tx* self,
                         enum allocator_op op, void* ptr,
                         struct picotm_error* error);

void
allocator_tx_undo_event(struct allocator_tx* self,
                        enum allocator_op op, void* ptr,
                        struct picotm_error* error);

void
allocator_tx_finish(struct allocator_tx* self);
