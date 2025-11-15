/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

#include "malloc_usable_size.h"
#if defined(HAVE_MALLOC_MALLOC_H) && HAVE_MALLOC_MALLOC_H
#include <malloc/malloc.h>
#endif

size_t
__picotm_libc_malloc_usable_size(void* ptr)
{
#if (!defined(HAVE_DECL_MALLOC_USABLE_SIZE) || \
        !HAVE_DECL_MALLOC_USABLE_SIZE) && \
    (defined(HAVE_DECL_MALLOC_SIZE) && HAVE_DECL_MALLOC_SIZE)
    return malloc_size(ptr);
#else
    return 0;
#endif
}
