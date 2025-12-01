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
#include "picotm/picotm-txlist-state.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct txlist_entry*
txlist_state_begin(const struct txlist_state* self);

struct txlist_entry*
txlist_state_end(const struct txlist_state* self);

size_t
txlist_state_size(const struct txlist_state* self);

bool
txlist_state_is_empty(const struct txlist_state* self);

struct txlist_entry*
txlist_state_front(struct txlist_state* self);

struct txlist_entry*
txlist_state_back(struct txlist_state* self);

void
txlist_state_insert(struct txlist_state* self, struct txlist_entry* entry,
                    struct txlist_entry* position);

void
txlist_state_erase(struct txlist_state* self, struct txlist_entry* entry);
