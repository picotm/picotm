/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
#include "txqueue_entry.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

/**
 * \brief A queue transaction
 */
struct txqueue_tx {
    struct txqueue_entry local_head;
    struct txqueue_state* queue_state;
    struct txlib_tx* tx;
    struct picotm_rwstate state;
};

void
txqueue_tx_init(struct txqueue_tx* self, struct txqueue_state* queue_state,
                struct txlib_tx* tx);

void
txqueue_tx_uninit(struct txqueue_tx* self);

/*
 * Test for queue emptiness
 */

bool
txqueue_tx_exec_empty(struct txqueue_tx* self, struct picotm_error* error);

/*
 * Queue size
 */

size_t
txqueue_tx_exec_size(struct txqueue_tx* self, struct picotm_error* error);

/*
 * Front-entry access
 */

struct txqueue_entry*
txqueue_tx_exec_front(struct txqueue_tx* self, struct picotm_error* error);

/*
 * Back-entry access
 */

struct txqueue_entry*
txqueue_tx_exec_back(struct txqueue_tx* self, struct picotm_error* error);

/*
 * Remove from queue
 */

void
txqueue_tx_exec_pop(struct txqueue_tx* self, struct picotm_error* error);

void
txqueue_tx_undo_pop(struct txqueue_tx* self, struct txqueue_entry* entry,
                    bool use_local_queue, struct picotm_error* error);

/*
 * Append to queue
 */

void
txqueue_tx_exec_push(struct txqueue_tx* self, struct txqueue_entry* entry,
                     struct picotm_error* error);

void
txqueue_tx_apply_push(struct txqueue_tx* self, struct txqueue_entry* entry,
                      struct picotm_error* error);

void
txqueue_tx_undo_push(struct txqueue_tx* self, struct txqueue_entry* entry,
                     struct picotm_error* error);

/*
 * Module interface
 */

void
txqueue_tx_lock(struct txqueue_tx* self, struct picotm_error* error);

void
txqueue_tx_finish(struct txqueue_tx* self);
