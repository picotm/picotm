/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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
