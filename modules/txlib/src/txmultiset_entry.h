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
