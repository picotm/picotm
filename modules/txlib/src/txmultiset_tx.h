/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <picotm/picotm-lib-rwstate.h>
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
