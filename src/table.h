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

#pragma once

#include <stddef.h>

struct picotm_error;

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

/**
 * Allocate or resize table.
 */
void*
tabresize(void* restrict base, size_t nelems, size_t newnelems, size_t siz,
          struct picotm_error* error);

/**
 * Free table memory.
 */
void
tabfree(void* restrict base);

/* Walk over table elements
 */

size_t
tabwalk_1(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, struct picotm_error*),
          struct picotm_error* error);

size_t
tabwalk_2(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, void*, struct picotm_error*), void* data,
          struct picotm_error* error);

size_t
tabwalk_3(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, void*, void*, struct picotm_error*),
          void* data1, void* data2,  struct picotm_error* error);

/* Walk over table in reversed order
 */

size_t
tabrwalk_1(void* restrict base, size_t nelems, size_t siz,
           size_t (*walk)(void*, struct picotm_error*),
           struct picotm_error* error);

size_t
tabrwalk_2(void* restrict base, size_t nelems, size_t siz,
           size_t (*walk)(void*, void*, struct picotm_error*), void* data,
           struct picotm_error* error);

/**
 * Filter out duplicate elements.
 * \return New length
 */
size_t
tabuniq(void* restrict base, size_t nelems, size_t siz,
        int (*compare)(const void*, const void*));
