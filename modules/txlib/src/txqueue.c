/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

#include "txqueue.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txqueue_tx.h"

PICOTM_EXPORT
void
txqueue_entry_init_tm(struct txqueue_entry* self)
{
    txqueue_entry_init(self);
}

PICOTM_EXPORT
void
txqueue_entry_uninit_tm(struct txqueue_entry* self)
{
    txqueue_entry_uninit(self);
}

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
