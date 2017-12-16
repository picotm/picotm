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

#include "txqueue_entry.h"
#include <assert.h>

PICOTM_EXPORT
void
txqueue_entry_init(struct txqueue_entry* self)
{
    assert(self);

    self->internal.next = NULL;
    self->internal.prev = NULL;
}

void
txqueue_entry_init_head(struct txqueue_entry* self)
{
    assert(self);

    /* The queue head refers to itself. */
    self->internal.next = self;
    self->internal.prev = self;
}

PICOTM_EXPORT
void
txqueue_entry_uninit(struct txqueue_entry* self)
{
    assert(self);
    assert(!txqueue_entry_is_enqueued(self));
}

void
txqueue_entry_uninit_head(struct txqueue_entry* self)
{
    assert(self);
    assert(txqueue_entry_next(self) == self);
    assert(txqueue_entry_prev(self) == self);
}

struct txqueue_entry*
txqueue_entry_next(const struct txqueue_entry* self)
{
    assert(self);

    return self->internal.next;
}

struct txqueue_entry*
txqueue_entry_prev(const struct txqueue_entry* self)
{
    assert(self);

    return self->internal.prev;
}

bool
txqueue_entry_is_enqueued(const struct txqueue_entry* self)
{
    assert(self);
    assert(!self->internal.next == !self->internal.prev);

    return !!self->internal.next;
}

void
txqueue_entry_insert(struct txqueue_entry* self, struct txqueue_entry* next)
{
    assert(self);
    assert(!txqueue_entry_is_enqueued(self));
    assert(txqueue_entry_is_enqueued(next));

    struct txqueue_entry* prev = txqueue_entry_prev(next);

    self->internal.prev = prev;
    self->internal.next = next;

    prev->internal.next = self;
    next->internal.prev = self;
}

void
txqueue_entry_erase(struct txqueue_entry* self)
{
    assert(self);
    assert(txqueue_entry_is_enqueued(self));

    struct txqueue_entry* next = txqueue_entry_next(self);
    struct txqueue_entry* prev = txqueue_entry_prev(self);

    next->internal.prev = prev;
    prev->internal.next = next;

    self->internal.next = NULL;
    self->internal.prev = NULL;
}

/*
 * Queue-entry helpers
 */

size_t
txqueue_entry_distance(const struct txqueue_entry* beg,
                       const struct txqueue_entry* end)
{
    size_t n = 0;

    for (; beg != end; beg = txqueue_entry_next(beg)) {
        ++n;
    }

    return n;
}
