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
#include "picotm/picotm-txstack-state.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct txstack_entry*
txstack_state_begin(struct txstack_state* self);

struct txstack_entry*
txstack_state_end(struct txstack_state* self);

bool
txstack_state_is_empty(struct txstack_state* self);

size_t
txstack_state_size(struct txstack_state* self);

struct txstack_entry*
txstack_state_top(struct txstack_state* self);

void
txstack_state_push(struct txstack_state* self,
                   struct txstack_entry* entry);

void
txstack_state_pop(struct txstack_state* self);
