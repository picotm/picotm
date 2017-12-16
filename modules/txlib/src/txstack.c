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

#include "txstack.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txstack_tx.h"

static struct txstack*
stack_of_stack_tx(struct txstack_tx* stack_tx)
{
    return (struct txstack*)stack_tx;
}

static struct txstack_tx*
stack_tx_of_stack(struct txstack* stack)
{
    return (struct txstack_tx*)stack;
}

PICOTM_EXPORT
struct txstack*
txstack_of_state_tx(struct txstack_state* stack_state)
{
retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txstack_tx* stack_tx =
            txlib_module_acquire_txstack_of_state(stack_state, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return stack_of_stack_tx(stack_tx);
    }
}

/*
 * Capacity
 */

PICOTM_EXPORT
bool
txstack_empty_tx(struct txstack* self)
{
    struct txstack_tx* stack_tx = stack_tx_of_stack(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        bool is_empty = txstack_tx_exec_empty(stack_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return is_empty;
    }
}

PICOTM_EXPORT
size_t
txstack_size_tx(struct txstack* self)
{
    struct txstack_tx* stack_tx = stack_tx_of_stack(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        size_t siz = txstack_tx_exec_size(stack_tx, &error);
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
struct txstack_entry*
txstack_top_tx(struct txstack* self)
{
    struct txstack_tx* stack_tx = stack_tx_of_stack(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txstack_entry* entry = txstack_tx_exec_top(stack_tx, &error);
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
txstack_pop_tx(struct txstack* self)
{
    struct txstack_tx* stack_tx = stack_tx_of_stack(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txstack_tx_exec_pop(stack_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txstack_push_tx(struct txstack* self, struct txstack_entry* entry)
{
    struct txstack_tx* stack_tx = stack_tx_of_stack(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txstack_tx_exec_push(stack_tx, entry, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}
