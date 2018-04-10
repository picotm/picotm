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

#include "txqueue_tx.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include "txqueue_state.h"
#include "txlib_event.h"
#include "txlib_tx.h"

void
txqueue_tx_init(struct txqueue_tx* self, struct txqueue_state* queue_state,
                struct txlib_tx* tx)
{
    assert(self);

    txqueue_entry_init_head(&self->local_head);

    self->queue_state = queue_state;
    self->tx = tx;

    picotm_rwstate_init(&self->state);
}

void
txqueue_tx_uninit(struct txqueue_tx* self)
{
    assert(self);

    picotm_rwstate_uninit(&self->state);
    txqueue_entry_uninit_head(&self->local_head);
}

static struct txqueue_entry*
local_begin(struct txqueue_tx* self)
{
    assert(self);

    return txqueue_entry_next(&self->local_head);
}

static struct txqueue_entry*
local_end(struct txqueue_tx* self)
{
    assert(self);

    return &self->local_head;
}

static bool
local_is_empty(struct txqueue_tx* self)
{
    return local_begin(self) == local_end(self);
}

static struct txqueue_entry*
local_front(struct txqueue_tx* self)
{
    assert(self);

    return local_begin(self);
}

static struct txqueue_entry*
local_back(struct txqueue_tx* self)
{
    return txqueue_entry_prev(local_end(self));
}

/*
 * Test for queue emptiness
 */

bool
txqueue_tx_exec_empty(struct txqueue_tx* self, struct picotm_error* error)
{
    assert(self);

    if (!local_is_empty(self)) {
        return false; /* has local entries; no need to check global state */
    }

    picotm_rwstate_try_rdlock(&self->state,
                              &self->queue_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    return txqueue_state_is_empty(self->queue_state);
}

/*
 * Queue size
 */

size_t
txqueue_tx_exec_size(struct txqueue_tx* self, struct picotm_error* error)
{
    size_t local_size = txqueue_entry_distance(local_begin(self),
                                               local_end(self));

    picotm_rwstate_try_rdlock(&self->state,
                              &self->queue_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return local_size + txqueue_state_size(self->queue_state);
}

/*
 * Front-entry access
 */

struct txqueue_entry*
txqueue_tx_exec_front(struct txqueue_tx* self, struct picotm_error* error)
{
    picotm_rwstate_try_rdlock(&self->state,
                              &self->queue_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    if (txqueue_state_is_empty(self->queue_state)) {
        return local_front(self);
    }

    return txqueue_state_front(self->queue_state);
}

/*
 * Back-entry access
 */

struct txqueue_entry*
txqueue_tx_exec_back(struct txqueue_tx* self, struct picotm_error* error)
{
    if (!local_is_empty(self)) {
        return local_back(self);
    }

    picotm_rwstate_try_rdlock(&self->state,
                              &self->queue_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return txqueue_state_back(self->queue_state);
}

/*
 * Remove from queue
 */

static void
exec_queue_pop(struct txlib_event* event, struct txqueue_tx* queue_tx,
               bool use_local_queue, struct picotm_error* error)
{
    assert(event);
    assert(queue_tx);

    struct txqueue_entry* entry;

    if (use_local_queue) {
        entry = local_front(queue_tx);
        txqueue_entry_erase(entry);
    } else {
        entry = txqueue_state_front(queue_tx->queue_state);
        txqueue_state_pop_front(queue_tx->queue_state);
    }

    event->op = TXLIB_QUEUE_POP;
    event->arg.queue_pop.queue_tx = queue_tx;
    event->arg.queue_pop.entry = entry;
    event->arg.queue_pop.use_local_queue = use_local_queue;
}

static void
init_queue_pop(struct txlib_event* event, void* data1, void* data2,
               struct picotm_error* error)
{
    exec_queue_pop(event, data1, *((bool*)data2), error);
}

void
txqueue_tx_exec_pop(struct txqueue_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->queue_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    bool use_local_queue = txqueue_state_is_empty(self->queue_state);

    if (!use_local_queue) {

        /* We only need a write lock if there are entries in the
         * shared queue state. */

        picotm_rwstate_try_wrlock(&self->state,
                                  &self->queue_state->internal.lock,
                                  error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    txlib_tx_append_events2(self->tx, 1, init_queue_pop, self,
                            &use_local_queue, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txqueue_tx_undo_pop(struct txqueue_tx* self, struct txqueue_entry* entry,
                    bool use_local_queue, struct picotm_error* error)
{
    assert(self);

    if (use_local_queue) {
        txqueue_entry_insert(entry, local_begin(self));
    } else {
        txqueue_state_push_front(self->queue_state, entry);
    }
}

/*
 * Prepend to queue
 */

static void
exec_queue_push(struct txlib_event* event, struct txqueue_tx* queue_tx,
                struct txqueue_entry* entry, struct picotm_error* error)
{
    assert(event);

    event->op = TXLIB_QUEUE_PUSH;
    event->arg.queue_push.queue_tx = queue_tx;
    event->arg.queue_push.entry = entry;

    txqueue_entry_insert(entry, local_end(queue_tx));
}

static void
init_queue_push(struct txlib_event* event, void* data1, void* data2,
                struct picotm_error* error)
{
    exec_queue_push(event, data1, data2, error);
}

void
txqueue_tx_exec_push(struct txqueue_tx* self, struct txqueue_entry* entry,
                     struct picotm_error* error)
{
    assert(self);

    txlib_tx_append_events2(self->tx, 1, init_queue_push, self, entry, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txqueue_tx_apply_push(struct txqueue_tx* self, struct txqueue_entry* entry,
                      struct picotm_error* error)
{
    assert(self);

    if (entry != local_front(self)) {
        /* The entry was pushed to the local queue, but might have later
         * been popped by the transaction's execute phase. Therefore, if
         * the next element to pop is not the entry, we return here. */
        return;
    }

    /* move entry from transaction-local queue to shared state */
    txqueue_entry_erase(entry);
    txqueue_state_push_back(self->queue_state, entry);
}

void
txqueue_tx_undo_push(struct txqueue_tx* self, struct txqueue_entry* entry,
                     struct picotm_error* error)
{
    txqueue_entry_erase(entry);
}

/*
 * Module interface
 */

void
txqueue_tx_lock(struct txqueue_tx* self, struct picotm_error* error)
{
    assert(self);

    if (local_is_empty(self)) {
        return;
    }

    picotm_rwstate_try_wrlock(&self->state,
                              &self->queue_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txqueue_tx_finish(struct txqueue_tx* self)
{
    assert(self);

    picotm_rwstate_unlock(&self->state, &self->queue_state->internal.lock);
}
