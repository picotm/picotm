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

#include "txlist_state.h"
#include <assert.h>
#include <stddef.h>
#include "txlist_entry.h"

PICOTM_EXPORT
void
txlist_state_init(struct txlist_state* self)
{
    assert(self);

    txlist_entry_init_head(&self->internal.head);
    picotm_rwlock_init(&self->internal.lock);
}

PICOTM_EXPORT
void
txlist_state_uninit(struct txlist_state* self)
{
    assert(self);

    picotm_rwlock_uninit(&self->internal.lock);
    txlist_entry_uninit_head(&self->internal.head);
}

PICOTM_EXPORT
void
txlist_state_clear_and_uninit_entries(struct txlist_state* self,
                                      void (*uninit)(struct txlist_entry*,
                                                     void*),
                                      void* data)
{
    assert(self);
    assert(uninit);

    struct txlist_entry* beg = txlist_state_begin(self);
    struct txlist_entry* end = txlist_state_end(self);

    for (; beg != end; beg = txlist_state_begin(self)) {
        txlist_state_erase(self, beg);
        uninit(beg, data);
    }
}

struct txlist_entry*
txlist_state_begin(const struct txlist_state* self)
{
    assert(self);

    return txlist_entry_next(&self->internal.head);
}

struct txlist_entry*
txlist_state_end(const struct txlist_state* self)
{
    assert(self);

    return (struct txlist_entry*)&self->internal.head;
}

size_t
txlist_state_size(const struct txlist_state* self)
{
    return txlist_entry_distance(txlist_state_begin(self),
                                 txlist_state_end(self));
}

bool
txlist_state_is_empty(const struct txlist_state* self)
{
    return txlist_state_begin(self) ==
           txlist_state_end(self);
}

struct txlist_entry*
txlist_state_front(struct txlist_state* self)
{
    return txlist_state_begin(self);
}

struct txlist_entry*
txlist_state_back(struct txlist_state* self)
{
    return txlist_entry_prev(txlist_state_end(self));
}

void
txlist_state_insert(struct txlist_state* self, struct txlist_entry* entry,
                    struct txlist_entry* position)
{
    txlist_entry_insert(entry, position);
}

void
txlist_state_erase(struct txlist_state* self, struct txlist_entry* entry)
{
    assert(entry != txlist_state_end(self));

    txlist_entry_erase(entry);
}
