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
#include "picotm/picotm-txqueue-state.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

void
txqueue_entry_init_head(struct txqueue_entry* self);

void
txqueue_entry_uninit_head(struct txqueue_entry* self);

struct txqueue_entry*
txqueue_entry_next(const struct txqueue_entry* self);

struct txqueue_entry*
txqueue_entry_prev(const struct txqueue_entry* self);

bool
txqueue_entry_is_enqueued(const struct txqueue_entry* self);

void
txqueue_entry_insert(struct txqueue_entry* self, struct txqueue_entry* next);

void
txqueue_entry_erase(struct txqueue_entry* self);

/*
 * Queue-entry helpers
 */

size_t
txqueue_entry_distance(const struct txqueue_entry* beg,
                       const struct txqueue_entry* end);
