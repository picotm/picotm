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

#include "txstack.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txstack_tx.h"

PICOTM_EXPORT
void
txstack_entry_init_tm(struct txstack_entry* self)
{
    txstack_entry_init(self);
}

PICOTM_EXPORT
void
txstack_entry_uninit_tm(struct txstack_entry* self)
{
    txstack_entry_uninit(self);
}

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
