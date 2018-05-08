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

#include "txlib_module.h"
#include "picotm/picotm-lib-spinlock.h"
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

    /* True if module structure has been initialized, false otherwise */
    bool is_initialized;
};

static void
prepare_commit(struct txlib_module* module, struct picotm_error* error)
{
    txlib_tx_prepare_commit(&module->tx, error);
}

static void
apply_event(struct txlib_module* module, uint16_t head, uintptr_t tail,
            struct picotm_error* error)
{
    txlib_tx_apply_event(&module->tx, head, tail, error);
}

static void
undo_event(struct txlib_module* module, uint16_t head, uintptr_t tail,
           struct picotm_error* error)
{
    txlib_tx_undo_event(&module->tx, head, tail, error);
}

static void
finish(struct txlib_module* module, struct picotm_error* error)
{
    txlib_tx_finish(&module->tx);
}

static void
uninit(struct txlib_module* module)
{
    txlib_tx_uninit(&module->tx);
    module->is_initialized = false;
}

/*
 * Thread-local data
 */

static void
prepare_commit_cb(void* data, int is_irrevocable, struct picotm_error* error)
{
    prepare_commit(data, error);
}

static void
apply_event_cb(uint16_t head, uintptr_t tail, void* data,
               struct picotm_error* error)
{
    apply_event(data, head, tail, error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void* data,
              struct picotm_error* error)
{
    undo_event(data, head, tail, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    finish(data, error);
}

static void
uninit_cb(void* data)
{
    uninit(data);
}

static struct txlib_tx*
get_txl_tx(bool initialize, struct picotm_error* error)
{
    static const struct picotm_module_ops g_ops = {
        .prepare_commit = prepare_commit_cb,
        .apply_event = apply_event_cb,
        .undo_event = undo_event_cb,
        .finish = finish_cb,
        .uninit = uninit_cb
    };
    static __thread struct txlib_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(&g_ops, &t_module, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txlib_tx_init(&t_module.tx, module);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct txlib_tx*
get_non_null_txl_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct txlib_tx* txl_tx = get_txl_tx(true, &error);

        if (!picotm_error_is_set(&error)) {
            assert(txl_tx);
            return txl_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

/*
 * Public interface
 */

struct txlist_tx*
txlib_module_acquire_txlist_of_state(struct txlist_state* list_state,
                                     struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_non_null_txl_tx();

    struct txlist_tx* list_tx =
        txlib_tx_acquire_txlist_of_state(txl_tx, list_state, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return list_tx;
}

struct txmultiset_tx*
txlib_module_acquire_txmultiset_of_state(
    struct txmultiset_state* multiset_state,
    struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_non_null_txl_tx();

    struct txmultiset_tx* multiset_tx =
        txlib_tx_acquire_txmultiset_of_state(txl_tx, multiset_state, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return multiset_tx;
}

struct txqueue_tx*
txlib_module_acquire_txqueue_of_state(struct txqueue_state* queue_state,
                                      struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_non_null_txl_tx();

    struct txqueue_tx* queue_tx =
        txlib_tx_acquire_txqueue_of_state(txl_tx, queue_state, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return queue_tx;
}

struct txstack_tx*
txlib_module_acquire_txstack_of_state(struct txstack_state* stack_state,
                                      struct picotm_error* error)
{
    struct txlib_tx* txl_tx = get_non_null_txl_tx();

    struct txstack_tx* stack_tx =
        txlib_tx_acquire_txstack_of_state(txl_tx, stack_state, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return stack_tx;
}
