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

#include "picotm/picotm-lib-rwstate.h"
#include "txstack_entry.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

/**
 * \brief A stack transaction
 */
struct txstack_tx {
    struct txstack_entry local_head;
    struct txstack_state* stack_state;
    struct txlib_tx* tx;
    struct picotm_rwstate state;
};

void
txstack_tx_init(struct txstack_tx* self, struct txstack_state* stack_state,
                struct txlib_tx* tx);

void
txstack_tx_uninit(struct txstack_tx* self);

/*
 * Test for stack emptiness
 */

bool
txstack_tx_exec_empty(struct txstack_tx* self, struct picotm_error* error);

/*
 * Stack size
 */

size_t
txstack_tx_exec_size(struct txstack_tx* self, struct picotm_error* error);

/*
 * Top-entry access
 */

struct txstack_entry*
txstack_tx_exec_top(struct txstack_tx* self, struct picotm_error* error);

/*
 * Remove from stack
 */

void
txstack_tx_exec_pop(struct txstack_tx* self, struct picotm_error* error);

void
txstack_tx_undo_pop(struct txstack_tx* self, struct txstack_entry* entry,
                    bool use_local_stack, struct picotm_error* error);

/*
 * Put onto stack
 */

void
txstack_tx_exec_push(struct txstack_tx* self, struct txstack_entry* entry,
                     struct picotm_error* error);

void
txstack_tx_apply_push(struct txstack_tx* self, struct txstack_entry* entry,
                      struct picotm_error* error);

void
txstack_tx_undo_push(struct txstack_tx* self, struct txstack_entry* entry,
                     struct picotm_error* error);

/*
 * Module interface
 */

void
txstack_tx_lock(struct txstack_tx* self, struct picotm_error* error);

void
txstack_tx_finish(struct txstack_tx* self);
