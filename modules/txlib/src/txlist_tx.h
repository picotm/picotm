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
#include <stddef.h>
#include <stdbool.h>

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct txlib_tx;
struct txlist_entry;
struct txlist_state;

/**
 * \brief A list transaction.
 */
struct txlist_tx {
    struct txlist_state* list_state;
    struct txlib_tx* tx;
    struct picotm_rwstate state;
};

void
txlist_tx_init(struct txlist_tx* self, struct txlist_state* list_state,
               struct txlib_tx* tx);

void
txlist_tx_uninit(struct txlist_tx* self);

/*
 * Begin iteration
 */

struct txlist_entry*
txlist_tx_exec_begin(struct txlist_tx* self, struct picotm_error* error);

/*
 * End iteration
 */

struct txlist_entry*
txlist_tx_exec_end(struct txlist_tx* self);

/*
 * Test for list emptiness
 */

bool
txlist_tx_exec_empty(struct txlist_tx* self, struct picotm_error* error);

/*
 * List size
 */

size_t
txlist_tx_exec_size(struct txlist_tx* self, struct picotm_error* error);

/*
 * Front-end entry
 */

struct txlist_entry*
txlist_tx_exec_front(struct txlist_tx* self, struct picotm_error* error);

/*
 * Back-end entry
 */

struct txlist_entry*
txlist_tx_exec_back(struct txlist_tx* self, struct picotm_error* error);

/*
 * Insert into list
 */

void
txlist_tx_exec_insert(struct txlist_tx* self, struct txlist_entry* entry,
                      struct txlist_entry* position,
                      struct picotm_error* error);

void
txlist_tx_undo_insert(struct txlist_tx* self, struct txlist_entry* entry,
                      struct picotm_error* error);

/*
 * Remove from list
 */

void
txlist_tx_exec_erase(struct txlist_tx* self, struct txlist_entry* entry,
                     struct picotm_error* error);

void
txlist_tx_undo_erase(struct txlist_tx* self, struct txlist_entry* entry,
                     struct txlist_entry* position,
                     struct picotm_error* error);

/*
 * Remove first entry
 */

void
txlist_tx_exec_pop_front(struct txlist_tx* self, struct picotm_error* error);

/*
 * Remove last entry
 */

void
txlist_tx_exec_pop_back(struct txlist_tx* self, struct picotm_error* error);

/*
 * Prepend to list
 */

void
txlist_tx_exec_push_front(struct txlist_tx* self, struct txlist_entry* entry,
                          struct picotm_error* error);

/*
 * Append to list
 */

void
txlist_tx_exec_push_back(struct txlist_tx* self, struct txlist_entry* entry,
                         struct picotm_error* error);

/*
 * Clear whole list
 */

void
txlist_tx_exec_clear(struct txlist_tx* self, struct picotm_error* error);

/*
 * Module interface
 */

void
txlist_tx_finish(struct txlist_tx* self);
