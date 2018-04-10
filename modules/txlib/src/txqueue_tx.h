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
