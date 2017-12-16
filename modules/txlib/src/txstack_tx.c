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

#include "txstack_tx.h"
#include <assert.h>
#include <picotm/picotm-module.h>
#include "txstack_state.h"
#include "txlib_event.h"
#include "txlib_tx.h"

void
txstack_tx_init(struct txstack_tx* self, struct txstack_state* stack_state,
                struct txlib_tx* tx)
{
    assert(self);

    txstack_entry_init_head(&self->local_head);

    self->stack_state = stack_state;
    self->tx = tx;

    picotm_rwstate_init(&self->state);
}

void
txstack_tx_uninit(struct txstack_tx* self)
{
    assert(self);

    picotm_rwstate_uninit(&self->state);
    txstack_entry_uninit_head(&self->local_head);
}

static struct txstack_entry*
local_begin(struct txstack_tx* self)
{
    assert(self);

    return txstack_entry_prev(&self->local_head);
}

static struct txstack_entry*
local_end(struct txstack_tx* self)
{
    assert(self);

    return &self->local_head;
}

static struct txstack_entry*
local_next_of(struct txstack_tx* self, struct txstack_entry* entry)
{
    struct txstack_entry* next = local_end(self);
    struct txstack_entry* pos = local_begin(self);

    for (; pos != entry; pos = txstack_entry_prev(pos)) {
        next = pos;
    }

    return next;
}

static bool
local_is_empty(struct txstack_tx* self)
{
    return local_begin(self) == local_end(self);
}

static struct txstack_entry*
local_top(struct txstack_tx* self)
{
    return txstack_entry_prev(local_end(self));
}

/*
 * Test for stack emptiness
 */

bool
txstack_tx_exec_empty(struct txstack_tx* self, struct picotm_error* error)
{
    assert(self);

    if (!local_is_empty(self)) {
        return false; /* has local entries; no need to check global state */
    }

    picotm_rwstate_try_rdlock(&self->state,
                              &self->stack_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    return txstack_state_is_empty(self->stack_state);
}

/*
 * Stack size
 */

size_t
txstack_tx_exec_size(struct txstack_tx* self, struct picotm_error* error)
{
    size_t local_size = txstack_entry_distance(local_begin(self),
                                               local_end(self));

    picotm_rwstate_try_rdlock(&self->state,
                              &self->stack_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return local_size + txstack_state_size(self->stack_state);
}

/*
 * Top-entry access
 */

struct txstack_entry*
txstack_tx_exec_top(struct txstack_tx* self, struct picotm_error* error)
{
    if (!local_is_empty(self)) {
        return local_top(self);
    }

    picotm_rwstate_try_rdlock(&self->state,
                              &self->stack_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return txstack_state_top(self->stack_state);
}

/*
 * Remove from stack
 */

static void
exec_stack_pop(struct txlib_event* event, struct txstack_tx* stack_tx,
               bool use_local_stack, struct picotm_error* error)
{
    assert(event);
    assert(stack_tx);

    struct txstack_entry* entry;

    if (use_local_stack) {
        entry = local_top(stack_tx);
        txstack_entry_erase(entry, local_end(stack_tx));
    } else {
        entry = txstack_state_top(stack_tx->stack_state);
        txstack_state_pop(stack_tx->stack_state);
    }

    event->op = TXLIB_STACK_POP;
    event->arg.stack_pop.stack_tx = stack_tx;
    event->arg.stack_pop.entry = entry;
    event->arg.stack_pop.use_local_stack = use_local_stack;
}

static void
init_stack_pop(struct txlib_event* event, void* data1, void* data2,
               struct picotm_error* error)
{
    exec_stack_pop(event, data1, *((bool*)data2), error);
}

void
txstack_tx_exec_pop(struct txstack_tx* self, struct picotm_error* error)
{
    assert(self);

    bool use_local_stack = !local_is_empty(self);

    if (!use_local_stack) {

        /* We only need a lock if there are no entries
         * in the local stack. */

        picotm_rwstate_try_wrlock(&self->state,
                                  &self->stack_state->internal.lock,
                                  error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    txlib_tx_append_events2(self->tx, 1, init_stack_pop, self,
                            &use_local_stack, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txstack_tx_undo_pop(struct txstack_tx* self, struct txstack_entry* entry,
                    bool use_local_stack, struct picotm_error* error)
{
    assert(self);

    if (use_local_stack) {
        txstack_entry_insert(entry, local_end(self));
    } else {
        txstack_state_push(self->stack_state, entry);
    }
}

/*
 * Put onto stack
 */

static void
exec_stack_push(struct txlib_event* event, struct txstack_tx* stack_tx,
                struct txstack_entry* entry, struct picotm_error* error)
{
    assert(event);

    event->op = TXLIB_STACK_PUSH;
    event->arg.stack_push.stack_tx = stack_tx;
    event->arg.stack_push.entry = entry;

    txstack_entry_insert(entry, local_end(stack_tx));
}

static void
init_stack_push(struct txlib_event* event, void* data1, void* data2,
                struct picotm_error* error)
{
    exec_stack_push(event, data1, data2, error);
}

void
txstack_tx_exec_push(struct txstack_tx* self, struct txstack_entry* entry,
                     struct picotm_error* error)
{
    assert(self);

    txlib_tx_append_events2(self->tx, 1, init_stack_push, self, entry, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txstack_tx_apply_push(struct txstack_tx* self, struct txstack_entry* entry,
                      struct picotm_error* error)
{
    assert(self);

    if (txstack_entry_prev(entry) != local_end(self)) {
        /* The entry was pushed to the local stack, but might have later
         * been popped by the transaction's execute phase. Therefore, if
         * the last element to pop is not the entry, we return here. */
        return;
    }

    /* move entry from transaction-local stack to shared state */
    txstack_entry_erase(entry, local_next_of(self, entry));
    txstack_state_push(self->stack_state, entry);
}

void
txstack_tx_undo_push(struct txstack_tx* self, struct txstack_entry* entry,
                     struct picotm_error* error)
{
    assert(local_top(self) == entry);

    txstack_entry_erase(entry, local_end(self));
}

/*
 * Module interface
 */

void
txstack_tx_lock(struct txstack_tx* self, struct picotm_error* error)
{
    assert(self);

    if (local_is_empty(self)) {
        return;
    }

    picotm_rwstate_try_wrlock(&self->state,
                              &self->stack_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txstack_tx_finish(struct txstack_tx* self)
{
    assert(self);

    picotm_rwstate_unlock(&self->state, &self->stack_state->internal.lock);
}
