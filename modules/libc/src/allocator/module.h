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

#include <stddef.h>

struct picotm_error;

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \ingroup libc_impl
 * \ingroup libc_impl_allocator
 * \file
 * \endcond
 */

void
allocator_module_free(void* mem, size_t usiz, struct picotm_error* error);

void*
allocator_module_malloc(size_t size, struct picotm_error* error);

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
int
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size,
                                struct picotm_error* error);
#endif

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \defgroup libc_impl_allocator libc Allocator
 * \endcond
 */
