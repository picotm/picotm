/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
