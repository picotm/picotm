/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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
