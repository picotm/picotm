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

#include "txmultiset.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txmultiset_tx.h"

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
