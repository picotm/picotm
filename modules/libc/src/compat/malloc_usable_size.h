/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

/**
 * \brief Returns the size of a dynamically allocated buffer.
 *
 * This interface was taken from GNU.
 */
size_t
__picotm_libc_malloc_usable_size(void* ptr);

#if !defined(HAVE_DECL_MALLOC_USABLE_SIZE) || !HAVE_DECL_MALLOC_USABLE_SIZE
#define malloc_usable_size	__picotm_libc_malloc_usable_size
#endif
