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

#include "txlist_tx.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include "txlist_entry.h"
#include "txlist_state.h"
#include "txlib_event.h"
#include "txlib_tx.h"

void
txlist_tx_init(struct txlist_tx* self, struct txlist_state* list_state,
               struct txlib_tx* tx)
{
    assert(self);
    assert(list_state);
    assert(tx);

    picotm_rwstate_init(&self->state);
    self->list_state = list_state;
    self->tx = tx;
}

void
txlist_tx_uninit(struct txlist_tx* self)
{
    assert(self);

    picotm_rwstate_uninit(&self->state);
}

/*
 * Begin iteration
 */

struct txlist_entry*
txlist_tx_exec_begin(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return txlist_state_begin(self->list_state);
}

/*
 * End iteration
 */

struct txlist_entry*
txlist_tx_exec_end(struct txlist_tx* self)
{
    assert(self);

    return txlist_state_end(self->list_state);
}

/*
 * Test for list emptiness
 */

bool
txlist_tx_exec_empty(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return false;
    }

    return txlist_state_is_empty(self->list_state);
}

/*
 * List size
 */

size_t
txlist_tx_exec_size(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return txlist_state_size(self->list_state);
}

/*
 * Front-end entry
 */

struct txlist_entry*
txlist_tx_exec_front(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return txlist_state_front(self->list_state);
}

/*
 * Back-end entry
 */

struct txlist_entry*
txlist_tx_exec_back(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return txlist_state_back(self->list_state);
}

/*
 * Insert into list
 */

static void
exec_list_insert(struct txlib_event* event, struct txlist_tx* list_tx,
                 struct txlist_entry* entry, struct txlist_entry* position,
                 struct picotm_error* error)
{
    assert(event);

    event->op = TXLIB_LIST_INSERT;
    event->arg.list_insert.list_tx = list_tx;
    event->arg.list_insert.entry = entry;

    txlist_state_insert(list_tx->list_state, entry, position);
}

static void
init_list_insert(struct txlib_event* event, void* data1, void* data2,
                 void* data3, struct picotm_error* error)
{
    exec_list_insert(event, data1, data2, data3, error);
}

void
txlist_tx_exec_insert(struct txlist_tx* self, struct txlist_entry* entry,
                      struct txlist_entry* position,
                      struct picotm_error* error)
{
    assert(self);
    assert(entry);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events3(self->tx, 1, init_list_insert,
                            self, entry, position, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlist_tx_undo_insert(struct txlist_tx* self, struct txlist_entry* entry,
                      struct picotm_error* error)
{
    assert(self);

    txlist_state_erase(self->list_state, entry);
}

/*
 * Remove from list
 */

static void
exec_list_erase(struct txlib_event* event, struct txlist_tx* list_tx,
                struct txlist_entry* entry, struct picotm_error* error)
{
    assert(event);

    event->op = TXLIB_LIST_ERASE;
    event->arg.list_erase.list_tx = list_tx;
    event->arg.list_erase.entry = entry;
    event->arg.list_erase.position = txlist_entry_next(entry);

    txlist_state_erase(list_tx->list_state, entry);
}

static void
init_list_erase(struct txlib_event* event, void* data1, void* data2,
                struct picotm_error* error)
{
    exec_list_erase(event, data1, data2, error);
}

void
txlist_tx_exec_erase(struct txlist_tx* self, struct txlist_entry* entry,
                     struct picotm_error* error)
{
    assert(self);
    assert(entry);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events2(self->tx, 1, init_list_erase,
                            self, entry, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlist_tx_undo_erase(struct txlist_tx* self, struct txlist_entry* entry,
                     struct txlist_entry* position,
                     struct picotm_error* error)
{
    assert(self);

    txlist_state_insert(self->list_state, entry, position);
}

/*
 * Remove first entry
 */

static void
exec_list_pop_front(struct txlib_event* event, struct txlist_tx* list_tx,
                    struct picotm_error* error)
{
    assert(list_tx);

    exec_list_erase(event, list_tx, txlist_state_front(list_tx->list_state),
                    error);
}

static void
init_list_pop_front(struct txlib_event* event, void* data1,
                    struct picotm_error* error)
{
    exec_list_pop_front(event, data1, error);
}

void
txlist_tx_exec_pop_front(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events1(self->tx, 1, init_list_pop_front,
                            self, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Remove last entry
 */

static void
exec_list_pop_back(struct txlib_event* event, struct txlist_tx* list_tx,
                   struct picotm_error* error)
{
    assert(list_tx);

    exec_list_erase(event, list_tx, txlist_state_back(list_tx->list_state),
                    error);
}

static void
init_list_pop_back(struct txlib_event* event, void* data1,
                   struct picotm_error* error)
{
    exec_list_pop_back(event, data1, error);
}

void
txlist_tx_exec_pop_back(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events1(self->tx, 1, init_list_pop_back,
                            self, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Prepend to list
 */

static void
exec_list_push_front(struct txlib_event* event, struct txlist_tx* list_tx,
                     struct txlist_entry* entry, struct picotm_error* error)
{
    assert(list_tx);

    exec_list_insert(event, list_tx, entry,
                     txlist_state_begin(list_tx->list_state),
                     error);
}

static void
init_list_push_front(struct txlib_event* event, void* data1, void* data2,
                     struct picotm_error* error)
{
    exec_list_push_front(event, data1, data2, error);
}

void
txlist_tx_exec_push_front(struct txlist_tx* self, struct txlist_entry* entry,
                          struct picotm_error* error)
{
    assert(self);
    assert(entry);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events2(self->tx, 1, init_list_push_front,
                            self, entry, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Append to list
 */

static void
exec_list_push_back(struct txlib_event* event, struct txlist_tx* list_tx,
                    struct txlist_entry* entry, struct picotm_error* error)
{
    assert(list_tx);

    exec_list_insert(event, list_tx, entry,
                     txlist_state_end(list_tx->list_state),
                     error);
}

static void
init_list_push_back(struct txlib_event* event, void* data1, void* data2,
                    struct picotm_error* error)
{
    exec_list_push_back(event, data1, data2, error);
}

void
txlist_tx_exec_push_back(struct txlist_tx* self, struct txlist_entry* entry,
                         struct picotm_error* error)
{
    assert(self);
    assert(entry);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_tx_append_events2(self->tx, 1, init_list_push_back,
                            self, entry, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Clear whole list
 */

static void
exec_list_clear(struct txlib_event* event, struct txlist_tx* list_tx,
                struct picotm_error* error)
{
    assert(event);

    exec_list_erase(event, list_tx,
                    txlist_state_front(list_tx->list_state),
                    error);
}

static void
init_list_clear(struct txlib_event* event, void* data1,
                struct picotm_error* error)
{
    exec_list_clear(event, data1, error);
}

void
txlist_tx_exec_clear(struct txlist_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_wrlock(&self->state,
                              &self->list_state->internal.lock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    size_t siz = txlist_state_size(self->list_state);

    txlib_tx_append_events1(self->tx, siz, init_list_clear, self, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Module interface
 */

void
txlist_tx_finish(struct txlist_tx* self)
{
    assert(self);

    picotm_rwstate_unlock(&self->state, &self->list_state->internal.lock);
}
