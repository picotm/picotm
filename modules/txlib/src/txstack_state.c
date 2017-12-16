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

#include "txstack_state.h"
#include <assert.h>
#include "txstack_entry.h"

PICOTM_EXPORT
void
txstack_state_init(struct txstack_state* self)
{
    assert(self);

    txstack_entry_init_head(&self->internal.head);
    picotm_rwlock_init(&self->internal.lock);
}

PICOTM_EXPORT
void
txstack_state_uninit(struct txstack_state* self)
{
    assert(self);
    assert(txstack_state_is_empty(self));

    txstack_entry_uninit_head(&self->internal.head);
    picotm_rwlock_uninit(&self->internal.lock);
}

PICOTM_EXPORT
void
txstack_state_clear_and_uninit_entries(struct txstack_state* self,
                                       void (*uninit)(struct txstack_entry*,
                                                      void*),
                                       void* data)
{
    assert(self);
    assert(uninit);

    struct txstack_entry* beg = txstack_state_begin(self);
    struct txstack_entry* end = txstack_state_end(self);

    for (; beg != end; beg = txstack_state_begin(self)) {
        txstack_entry_erase(beg, end);
        uninit(beg, data);
    }
}

bool
txstack_state_is_empty(struct txstack_state* self)
{
    return txstack_state_begin(self) == txstack_state_end(self);
}

size_t
txstack_state_size(struct txstack_state* self)
{
    return txstack_entry_distance(txstack_state_begin(self),
                                  txstack_state_end(self));
}

struct txstack_entry*
txstack_state_begin(struct txstack_state* self)
{
    assert(self);

    return txstack_entry_prev(&self->internal.head);
}

struct txstack_entry*
txstack_state_end(struct txstack_state* self)
{
    assert(self);

    return &self->internal.head;
}

struct txstack_entry*
txstack_state_top(struct txstack_state* self)
{
    return txstack_entry_prev(txstack_state_end(self));
}

void
txstack_state_push(struct txstack_state* self,
                   struct txstack_entry* entry)
{
    txstack_entry_insert(entry, txstack_state_end(self));
}

void
txstack_state_pop(struct txstack_state* self)
{
    txstack_entry_erase(txstack_state_top(self), txstack_state_end(self));
}
