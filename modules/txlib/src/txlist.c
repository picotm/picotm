/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "txlist.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txlist_tx.h"

PICOTM_EXPORT
void
txlist_entry_init_tm(struct txlist_entry* self)
{
    txlist_entry_init(self);
}

PICOTM_EXPORT
void
txlist_entry_uninit_tm(struct txlist_entry* self)
{
    txlist_entry_uninit(self);
}

static struct txlist*
list_of_list_tx(struct txlist_tx* list_tx)
{
    return (struct txlist*)list_tx;
}

static struct txlist_tx*
list_tx_of_list(struct txlist* list)
{
    return (struct txlist_tx*)list;
}

PICOTM_EXPORT
struct txlist*
txlist_of_state_tx(struct txlist_state* list_state)
{
retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txlist_tx* list_tx =
            txlib_module_acquire_txlist_of_state(list_state, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return list_of_list_tx(list_tx);
    }
}

/*
 * Entries
 */

PICOTM_EXPORT
struct txlist_entry*
txlist_begin_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txlist_entry* beg =
            txlist_tx_exec_begin(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return beg;
    }
}

PICOTM_EXPORT
struct txlist_entry*
txlist_end_tx(struct txlist* self)
{
    return txlist_tx_exec_end(list_tx_of_list(self));
}

/*
 * Element access
 */

PICOTM_EXPORT
struct txlist_entry*
txlist_front_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txlist_entry* entry = txlist_tx_exec_front(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return entry;
    }
}

PICOTM_EXPORT
struct txlist_entry*
txlist_back_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txlist_entry* entry = txlist_tx_exec_back(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return entry;
    }
}

/*
 * Capacity
 */

PICOTM_EXPORT
bool
txlist_empty_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        bool is_empty = txlist_tx_exec_empty(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return is_empty;
    }
}

PICOTM_EXPORT
size_t
txlist_size_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        size_t siz = txlist_tx_exec_size(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return siz;
    }
}

/*
 * Modifiers
 */

PICOTM_EXPORT
void
txlist_clear_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txlist_tx_exec_clear(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txlist_erase_tx(struct txlist* self, struct txlist_entry* entry)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txlist_tx_exec_erase(list_tx, entry, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txlist_insert_tx(struct txlist* self,
                 struct txlist_entry* entry,
                 struct txlist_entry* position)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txlist_tx_exec_insert(list_tx, entry, position, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txlist_pop_front_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txlist_tx_exec_pop_front(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txlist_pop_back_tx(struct txlist* self)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txlist_tx_exec_pop_back(list_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txlist_push_front_tx(struct txlist* self, struct txlist_entry* entry)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txlist_tx_exec_push_front(list_tx, entry, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txlist_push_back_tx(struct txlist* self, struct txlist_entry* entry)
{
    struct txlist_tx* list_tx = list_tx_of_list(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txlist_tx_exec_push_back(list_tx, entry, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}
