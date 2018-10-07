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

#include <stdbool.h>
#include <stddef.h>
#include "picotm/picotm-txlist-state.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

void
txlist_entry_init_head(struct txlist_entry* self);

void
txlist_entry_uninit_head(struct txlist_entry* self);

struct txlist_entry*
txlist_entry_next(const struct txlist_entry* self);

struct txlist_entry*
txlist_entry_prev(const struct txlist_entry* self);

bool
txlist_entry_is_enqueued(const struct txlist_entry* self);

void
txlist_entry_insert(struct txlist_entry* self, struct txlist_entry* next);

void
txlist_entry_erase(struct txlist_entry* self);

/*
 * List-entry helpers
 */

size_t
txlist_entry_distance(const struct txlist_entry* beg,
                      const struct txlist_entry* end);
