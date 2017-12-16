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

#include <stdbool.h>
#include <stddef.h>
#include "picotm/picotm-txmultiset-state.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

void
txmultiset_entry_init_head(struct txmultiset_entry* self);

void
txmultiset_entry_uninit_head(struct txmultiset_entry* self);

struct txmultiset_entry*
txmultiset_entry_begin(const struct txmultiset_entry* self);

struct txmultiset_entry*
txmultiset_entry_end(const struct txmultiset_entry* self);

struct txmultiset_entry*
txmultiset_entry_next(const struct txmultiset_entry* self);

struct txmultiset_entry*
txmultiset_entry_prev(const struct txmultiset_entry* self);

struct txmultiset_entry*
txmultiset_entry_parent(const struct txmultiset_entry* self);

bool
txmultiset_entry_is_enqueued(const struct txmultiset_entry* self);

void
txmultiset_entry_insert(struct txmultiset_entry* self,
                        struct txmultiset_entry* head,
                        int (*cmp_keys)(const void* lhs, const void* rhs),
                        const void* (*key_of_entry)(struct txmultiset_entry*));

void
txmultiset_entry_erase(struct txmultiset_entry* self);

struct txmultiset_entry*
txmultiset_entry_find(struct txmultiset_entry* self,
                      const void* key,
                      int (*cmp_keys)(const void* lhs, const void* rhs),
                      const void* (*key_of_entry)(struct txmultiset_entry*));

/*
 * Mulitset-entry helpers
 */

size_t
txmultiset_entry_distance(const struct txmultiset_entry* beg,
                          const struct txmultiset_entry* end);
