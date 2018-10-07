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
struct txmultiset_entry;
struct txmultiset_state;

/**
 * \brief A multiset transaction.
 */
struct txmultiset_tx {
    struct txmultiset_state* multiset_state;
    struct txlib_tx* tx;
    struct picotm_rwstate state;
};

void
txmultiset_tx_init(struct txmultiset_tx* self,
                   struct txmultiset_state* multiset_state,
                   struct txlib_tx* tx);

void
txmultiset_tx_uninit(struct txmultiset_tx* self);

/*
 * Begin iteration
 */

struct txmultiset_entry*
txmultiset_tx_exec_begin(struct txmultiset_tx* self,
                         struct picotm_error* error);

/*
 * End iteration
 */

struct txmultiset_entry*
txmultiset_tx_exec_end(struct txmultiset_tx* self);

/*
 * Test for multiset emptiness
 */

bool
txmultiset_tx_exec_empty(struct txmultiset_tx* self,
                         struct picotm_error* error);

/*
 * List size
 */

size_t
txmultiset_tx_exec_size(struct txmultiset_tx* self,
                        struct picotm_error* error);

/*
 * Insert into multiset
 */

void
txmultiset_tx_exec_insert(struct txmultiset_tx* self,
                          struct txmultiset_entry* entry,
                          struct picotm_error* error);

void
txmultiset_tx_undo_insert(struct txmultiset_tx* self,
                          struct txmultiset_entry* entry,
                          struct picotm_error* error);

/*
 * Remove from multiset
 */

void
txmultiset_tx_exec_erase(struct txmultiset_tx* self,
                         struct txmultiset_entry* entry,
                         struct picotm_error* error);

void
txmultiset_tx_undo_erase(struct txmultiset_tx* self,
                         struct txmultiset_entry* entry,
                         struct picotm_error* error);

/*
 * Clear whole multiset
 */

void
txmultiset_tx_exec_clear(struct txmultiset_tx* self,
                         struct picotm_error* error);

/*
 * Find entry in multiset
 */

struct txmultiset_entry*
txmultiset_tx_exec_find(struct txmultiset_tx* self, const void* key,
                        struct picotm_error* error);

/*
 * Get lower and upper bounding entries for a specific key
 */

struct txmultiset_entry*
txmultiset_tx_exec_lower_bound(struct txmultiset_tx* self, const void* key,
                               struct picotm_error* error);

struct txmultiset_entry*
txmultiset_tx_exec_upper_bound(struct txmultiset_tx* self, const void* key,
                               struct picotm_error* error);

/*
 * Count entries with a specific key
 */

size_t
txmultiset_tx_exec_count(struct txmultiset_tx* self, const void* key,
                         struct picotm_error* error);
/*
 * Module interface
 */

void
txmultiset_tx_finish(struct txmultiset_tx* self);
