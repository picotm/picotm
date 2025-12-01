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

#include "txmultiset.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txmultiset_tx.h"

PICOTM_EXPORT
void
txmultiset_entry_init_tm(struct txmultiset_entry* self)
{
    txmultiset_entry_init(self);
}

PICOTM_EXPORT
void
txmultiset_entry_uninit_tm(struct txmultiset_entry* self)
{
    txmultiset_entry_uninit(self);
}

static struct txmultiset*
multiset_of_multiset_tx(struct txmultiset_tx* multiset_tx)
{
    return (struct txmultiset*)multiset_tx;
}

static struct txmultiset_tx*
multiset_tx_of_multiset(struct txmultiset* multiset)
{
    return (struct txmultiset_tx*)multiset;
}

PICOTM_EXPORT
struct txmultiset*
txmultiset_of_state_tx(struct txmultiset_state* multiset_state)
{
retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txmultiset_tx* multiset_tx =
            txlib_module_acquire_txmultiset_of_state(multiset_state, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return multiset_of_multiset_tx(multiset_tx);
    }
}

/*
 * Entries
 */

PICOTM_EXPORT
struct txmultiset_entry*
txmultiset_begin_tx(struct txmultiset* self)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txmultiset_entry* beg =
            txmultiset_tx_exec_begin(multiset_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return beg;
    }
}

PICOTM_EXPORT
struct txmultiset_entry*
txmultiset_end_tx(struct txmultiset* self)
{
    return txmultiset_tx_exec_end(multiset_tx_of_multiset(self));
}

/*
 * Capacity
 */

PICOTM_EXPORT
bool
txmultiset_empty_tx(struct txmultiset* self)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        bool is_empty = txmultiset_tx_exec_empty(multiset_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return is_empty;
    }
}

PICOTM_EXPORT
size_t
txmultiset_size_tx(struct txmultiset* self)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        size_t siz = txmultiset_tx_exec_size(multiset_tx, &error);
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
txmultiset_clear_tx(struct txmultiset* self)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txmultiset_tx_exec_clear(multiset_tx, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txmultiset_erase_tx(struct txmultiset* self, struct txmultiset_entry* entry)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txmultiset_tx_exec_erase(multiset_tx, entry, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

PICOTM_EXPORT
void
txmultiset_insert_tx(struct txmultiset* self, struct txmultiset_entry* entry)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        txmultiset_tx_exec_insert(multiset_tx, entry, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
    }
}

/*
 * Operations
 */

PICOTM_EXPORT
struct txmultiset_entry*
txmultiset_find_tx(struct txmultiset* self, const void* key)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txmultiset_entry* entry = txmultiset_tx_exec_find(multiset_tx,
                                                                 key, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return entry;
    }
}

PICOTM_EXPORT
struct txmultiset_entry*
txmultiset_lower_bound_tx(struct txmultiset* self, const void* key)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txmultiset_entry* entry =
            txmultiset_tx_exec_lower_bound(multiset_tx, key, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return entry;
    }
}

PICOTM_EXPORT
struct txmultiset_entry*
txmultiset_upper_bound_tx(struct txmultiset* self, const void* key)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        struct txmultiset_entry* entry =
            txmultiset_tx_exec_upper_bound(multiset_tx, key, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return entry;
    }
}

PICOTM_EXPORT
size_t
txmultiset_count_tx(struct txmultiset* self, const void* key)
{
    struct txmultiset_tx* multiset_tx = multiset_tx_of_multiset(self);

retry:
    {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        size_t count = txmultiset_tx_exec_count(multiset_tx, key, &error);
        if (picotm_error_is_set(&error)) {
            picotm_recover_from_error(&error);
            goto retry;
        }
        return count;
    }
}
