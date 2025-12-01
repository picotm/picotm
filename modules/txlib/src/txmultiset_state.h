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

#include <stdbool.h>
#include <stddef.h>
#include "picotm/picotm-txmultiset-state.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct txmultiset_entry*
txmultiset_state_begin(const struct txmultiset_state* self);

struct txmultiset_entry*
txmultiset_state_end(const struct txmultiset_state* self);

size_t
txmultiset_state_size(const struct txmultiset_state* self);

bool
txmultiset_state_is_empty(const struct txmultiset_state* self);

struct txmultiset_entry*
txmultiset_state_front(struct txmultiset_state* self);

struct txmultiset_entry*
txmultiset_state_back(struct txmultiset_state* self);

void
txmultiset_state_insert(struct txmultiset_state* self,
                        struct txmultiset_entry* entry);

void
txmultiset_state_erase(struct txmultiset_state* self,
                       struct txmultiset_entry* entry);

struct txmultiset_entry*
txmultiset_state_find(struct txmultiset_state* self, const void* key);

struct txmultiset_entry*
txmultiset_state_lower_bound(struct txmultiset_state* self, const void* key);

struct txmultiset_entry*
txmultiset_state_upper_bound(struct txmultiset_state* self, const void* key);

size_t
txmultiset_state_count(struct txmultiset_state* self, const void* key);
