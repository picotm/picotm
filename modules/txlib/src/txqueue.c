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

#include "txqueue.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txqueue_tx.h"

static struct txqueue*
queue_of_queue_tx(struct txqueue_tx* queue_tx)
{
    return (struct txqueue*)queue_tx;
}

static struct txqueue_tx*
queue_tx_of_queue(struct txqueue* queue)
{
    return (struct txqueue_tx*)queue;
}

PICOTM_EXPORT
struct txqueue*
txqueue_of_state_tx(struct txqueue_state* queue_state)
{
retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txqueue_tx* queue_tx =
            txlib_module_acquire_txqueue_of_state(queue_state, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return queue_of_queue_tx(queue_tx);
    }
}

/*
 * Capacity
 */

PICOTM_EXPORT
bool
txqueue_empty_tx(struct txqueue* self)
{
    struct txqueue_tx* queue_tx = queue_tx_of_queue(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        bool is_empty = txqueue_tx_exec_empty(queue_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return is_empty;
    }
}

PICOTM_EXPORT
size_t
txqueue_size_tx(struct txqueue* self)
{
    struct txqueue_tx* queue_tx = queue_tx_of_queue(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        size_t siz = txqueue_tx_exec_size(queue_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return siz;
    }
}

/*
 * Access
 */

PICOTM_EXPORT
struct txqueue_entry*
txqueue_front_tx(struct txqueue* self)
{
    struct txqueue_tx* queue_tx = queue_tx_of_queue(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txqueue_entry* entry = txqueue_tx_exec_front(queue_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return entry;
    }
}

PICOTM_EXPORT
struct txqueue_entry*
txqueue_back_tx(struct txqueue* self)
{
    struct txqueue_tx* queue_tx = queue_tx_of_queue(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txqueue_entry* entry = txqueue_tx_exec_back(queue_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return entry;
    }
}

/*
 * Modfiers
 */

PICOTM_EXPORT
void
txqueue_pop_tx(struct txqueue* self)
{
    struct txqueue_tx* queue_tx = queue_tx_of_queue(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txqueue_tx_exec_pop(queue_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txqueue_push_tx(struct txqueue* self, struct txqueue_entry* entry)
{
    struct txqueue_tx* queue_tx = queue_tx_of_queue(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txqueue_tx_exec_push(queue_tx, entry, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}
