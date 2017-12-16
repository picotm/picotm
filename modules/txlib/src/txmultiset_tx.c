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
 */

#include "txmultiset_tx.h"
#include <assert.h>
#include <picotm/picotm-module.h>
#include "txmultiset_entry.h"
#include "txmultiset_state.h"
#include "txlib_event.h"
#include "txlib_tx.h"

void
txmultiset_tx_init(struct txmultiset_tx* self,
                   struct txmultiset_state* multiset_state,
                   struct txlib_tx* tx)
{
    assert(self);
    assert(multiset_state);
    assert(tx);

    picotm_rwstate_init(&self->state);
    self->multiset_state = multiset_state;
    self->tx = tx;
}

void
txmultiset_tx_uninit(struct txmultiset_tx* self)
{
    assert(self);

    picotm_rwstate_uninit(&self->state);
}

/*
 * Begin iteration
 */

struct txmultiset_entry*
txmultiset_tx_exec_begin(struct txmultiset_tx* self,
                         struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return txmultiset_state_begin(self->multiset_state);
}

/*
 * End iteration
 */

struct txmultiset_entry*
txmultiset_tx_exec_end(struct txmultiset_tx* self)
{
    assert(self);

    return txmultiset_state_end(self->multiset_state);
}

/*
 * Test for multiset emptiness
 */

bool
txmultiset_tx_exec_empty(struct txmultiset_tx* self,
                         struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    return txmultiset_state_is_empty(self->multiset_state);
}

/*
 * List size
 */

size_t
txmultiset_tx_exec_size(struct txmultiset_tx* self,
                        struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return txmultiset_state_size(self->multiset_state);
}

/*
 * Insert into multiset
 */

static void
exec_multiset_insert(struct txlib_event* event,
                     struct txmultiset_tx* multiset_tx,
                     struct txmultiset_entry* entry,
                     struct picotm_error* error)
{
    assert(event);

    event->op = TXLIB_MULTISET_INSERT;
    event->arg.multiset_insert.multiset_tx = multiset_tx;
    event->arg.multiset_insert.entry = entry;

    txmultiset_state_insert(multiset_tx->multiset_state, entry);
}

static void
init_multiset_insert(struct txlib_event* event, void* data1, void* data2,
                     struct picotm_error* error)
{
    exec_multiset_insert(event, data1, data2, error);
}

void
txmultiset_tx_exec_insert(struct txmultiset_tx* self,
                          struct txmultiset_entry* entry,
                          struct picotm_error* error)
{
    assert(self);
    assert(entry);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events2(self->tx, 1, init_multiset_insert, self, entry,
                            error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txmultiset_tx_undo_insert(struct txmultiset_tx* self,
                          struct txmultiset_entry* entry,
                          struct picotm_error* error)
{
    assert(self);

    txmultiset_state_erase(self->multiset_state, entry);
}

/*
 * Remove from multiset
 */

static void
exec_multiset_erase(struct txlib_event* event,
                    struct txmultiset_tx* multiset_tx,
                    struct txmultiset_entry* entry,
                    struct picotm_error* error)
{
    assert(event);

    event->op = TXLIB_MULTISET_ERASE;
    event->arg.multiset_erase.multiset_tx = multiset_tx;
    event->arg.multiset_erase.entry = entry;

    txmultiset_state_erase(multiset_tx->multiset_state, entry);
}

static void
init_multiset_erase(struct txlib_event* event, void* data1, void* data2,
                    struct picotm_error* error)
{
    exec_multiset_erase(event, data1, data2, error);
}

void
txmultiset_tx_exec_erase(struct txmultiset_tx* self,
                         struct txmultiset_entry* entry,
                         struct picotm_error* error)
{
    assert(self);
    assert(entry);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events2(self->tx, 1, init_multiset_erase,
                            self, entry, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txmultiset_tx_undo_erase(struct txmultiset_tx* self,
                         struct txmultiset_entry* entry,
                         struct picotm_error* error)
{
    assert(self);

    txmultiset_state_insert(self->multiset_state, entry);
}

/*
 * Clear whole multiset
 */

static void
exec_multiset_clear(struct txlib_event* event,
                    struct txmultiset_tx* multiset_tx,
                    struct picotm_error* error)
{
    assert(event);

    exec_multiset_erase(event, multiset_tx,
                        txmultiset_state_front(multiset_tx->multiset_state),
                        error);
}

static void
init_multiset_clear(struct txlib_event* event, void* data1,
                struct picotm_error* error)
{
    exec_multiset_clear(event, data1, error);
}

void
txmultiset_tx_exec_clear(struct txmultiset_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    size_t siz = txmultiset_state_size(self->multiset_state);

    txlib_tx_append_events1(self->tx, siz, init_multiset_clear, self, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Find entry in multiset
 */

struct txmultiset_entry*
txmultiset_tx_exec_find(struct txmultiset_tx* self, const void* key,
                        struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    return txmultiset_state_find(self->multiset_state, key);
}

/*
 * Get lower and upper bounding entries for a key
 */

struct txmultiset_entry*
txmultiset_tx_exec_lower_bound(struct txmultiset_tx* self, const void* key,
                               struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    return txmultiset_state_lower_bound(self->multiset_state, key);
}

struct txmultiset_entry*
txmultiset_tx_exec_upper_bound(struct txmultiset_tx* self, const void* key,
                               struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    return txmultiset_state_upper_bound(self->multiset_state, key);
}

size_t
txmultiset_tx_exec_count(struct txmultiset_tx* self, const void* key,
                         struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->multiset_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return txmultiset_state_count(self->multiset_state, key);
}

/*
 * Module interface
 */

void
txmultiset_tx_finish(struct txmultiset_tx* self)
{
    assert(self);

    picotm_rwstate_unlock(&self->state, &self->multiset_state->internal.lock);
}
