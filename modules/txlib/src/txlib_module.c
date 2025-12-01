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

#include "txlib_module.h"
#include "picotm/picotm-lib-state.h"
#include "picotm/picotm-lib-thread-state.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdlib.h>
#include "txlib_tx.h"
#include "txlist_tx.h"
#include "txmultiset_tx.h"
#include "txqueue_tx.h"
#include "txstack_tx.h"

/*
 * Module interface
 */

struct txlib_module {
    struct txlib_tx tx;
};

static void
txlib_module_init(struct txlib_module* self, unsigned long module_id)
{
    assert(self);

    txlib_tx_init(&self->tx, module_id);
}

static void
txlib_module_uninit(struct txlib_module* self)
{
    assert(self);

    txlib_tx_uninit(&self->tx);
}

static void
txlib_module_prepare_commit(struct txlib_module* self,
                            struct picotm_error* error)
{
    assert(self);

    txlib_tx_prepare_commit(&self->tx, error);
}

static void
txlib_module_apply_event(struct txlib_module* self, uint16_t head,
                         uintptr_t tail, struct picotm_error* error)
{
    assert(self);

    txlib_tx_apply_event(&self->tx, head, tail, error);
}

static void
txlib_module_undo_event(struct txlib_module* self, uint16_t head,
                        uintptr_t tail, struct picotm_error* error)
{
    assert(self);

    txlib_tx_undo_event(&self->tx, head, tail, error);
}

static void
txlib_module_finish(struct txlib_module* self)
{
    assert(self);

    txlib_tx_finish(&self->tx);
}

/*
 * Thread-local state
 */

PICOTM_STATE(txlib_module, struct txlib_module);
PICOTM_STATE_STATIC_DECL(txlib_module, struct txlib_module)
PICOTM_THREAD_STATE_STATIC_DECL(txlib_module)

static void
prepare_commit_cb(void* data, int is_irrevocable, struct picotm_error* error)
{
    struct txlib_module* module = data;
    txlib_module_prepare_commit(module, error);
}

static void
apply_event_cb(uint16_t head, uintptr_t tail, void* data,
               struct picotm_error* error)
{
    struct txlib_module* module = data;
    txlib_module_apply_event(module, head, tail, error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void* data,
              struct picotm_error* error)
{
    struct txlib_module* module = data;
    txlib_module_undo_event(module, head, tail, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct txlib_module* module = data;
    txlib_module_finish(module);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(txlib_module);
}

static void
init_txlib_module(struct txlib_module* module, struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .prepare_commit = prepare_commit_cb,
        .apply_event = apply_event_cb,
        .undo_event = undo_event_cb,
        .finish = finish_cb,
        .release = release_cb
    };

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    txlib_module_init(module, module_id);
}

static void
uninit_txlib_module(struct txlib_module* module)
{
    txlib_module_uninit(module);
}

PICOTM_STATE_STATIC_IMPL(txlib_module, struct txlib_module,
                         init_txlib_module,
                         uninit_txlib_module)
PICOTM_THREAD_STATE_STATIC_IMPL(txlib_module)

static struct txlib_tx*
get_txlib_tx(struct picotm_error* error)
{
    struct txlib_module* module = PICOTM_THREAD_STATE_ACQUIRE(txlib_module,
                                                              true, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    return &module->tx;
}

/*
 * Public interface
 */

struct txlist_tx*
txlib_module_acquire_txlist_of_state(struct txlist_state* list_state,
                                     struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_txlib_tx(error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    struct txlist_tx* list_tx =
        txlib_tx_acquire_txlist_of_state(txl_tx, list_state, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    return list_tx;
}

struct txmultiset_tx*
txlib_module_acquire_txmultiset_of_state(
    struct txmultiset_state* multiset_state,
    struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_txlib_tx(error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    struct txmultiset_tx* multiset_tx =
        txlib_tx_acquire_txmultiset_of_state(txl_tx, multiset_state, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    return multiset_tx;
}

struct txqueue_tx*
txlib_module_acquire_txqueue_of_state(struct txqueue_state* queue_state,
                                      struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_txlib_tx(error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    struct txqueue_tx* queue_tx =
        txlib_tx_acquire_txqueue_of_state(txl_tx, queue_state, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    return queue_tx;
}

struct txstack_tx*
txlib_module_acquire_txstack_of_state(struct txstack_state* stack_state,
                                      struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_txlib_tx(error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    struct txstack_tx* stack_tx =
        txlib_tx_acquire_txstack_of_state(txl_tx, stack_state, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }
    return stack_tx;
}
