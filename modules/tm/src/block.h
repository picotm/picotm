/*
 * picotm - A system-level transaction manager
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
#include <stdint.h>

/**
 * \cond impl || tm_impl
 * \ingroup tm_impl
 * \file
 * \endcond
 */

/* TODO: The block size should be independent from the properties of
 * malloc. This requires a clean-up in the page-handling algorithm.
 *
 * Power of 2; possibly length of cache lines, but no larger than
 * aligment and minimal size of malloc'ed memory. Otherwise we might
 * corrupt malloc's internal data structures.
 */
#define TM_BLOCK_SIZE_BITS      (3)
#define TM_BLOCK_SIZE           (1ul << TM_BLOCK_SIZE_BITS)
#define TM_BLOCK_OFFSET_MASK    ((1ul << TM_BLOCK_SIZE) - 1)
#define TM_BLOCK_INDEX_MASK     (~TM_BLOCK_FIELD_MASK)

static inline size_t
tm_block_index_at(uintptr_t addr)
{
    return addr / TM_BLOCK_SIZE;
}

static inline uintptr_t
tm_block_offset_at(uintptr_t addr)
{
    return addr & TM_BLOCK_OFFSET_MASK;
}
