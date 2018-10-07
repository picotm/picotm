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

struct txqueue_entry*
txqueue_state_begin(struct txqueue_state* self);

struct txqueue_entry*
txqueue_state_end(struct txqueue_state* self);

bool
txqueue_state_is_empty(struct txqueue_state* self);

size_t
txqueue_state_size(struct txqueue_state* self);

struct txqueue_entry*
txqueue_state_front(struct txqueue_state* self);

struct txqueue_entry*
txqueue_state_back(struct txqueue_state* self);

void
txqueue_state_push_front(struct txqueue_state* self,
                         struct txqueue_entry* entry);

void
txqueue_state_push_back(struct txqueue_state* self,
                        struct txqueue_entry* entry);

void
txqueue_state_pop_front(struct txqueue_state* self);
