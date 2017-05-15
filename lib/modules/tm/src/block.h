/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
#define TM_BLOCK_SIZE_BITS  (3)
#define TM_BLOCK_SIZE       (1ul << TM_BLOCK_SIZE_BITS)
#define TM_BLOCK_MASK       (~(TM_BLOCK_SIZE - 1))

static inline size_t
tm_block_index_at(uintptr_t addr)
{
    return addr / TM_BLOCK_SIZE;
}

static inline uintptr_t
tm_block_offset_at(uintptr_t addr)
{
    return addr & TM_BLOCK_MASK;
}
