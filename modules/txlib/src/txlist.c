/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#include "txlist.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"
#include "txlib_module.h"
#include "txlist_tx.h"

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
