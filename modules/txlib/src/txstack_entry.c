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
 */

#include "txstack_entry.h"
#include <assert.h>

PICOTM_EXPORT
void
txstack_entry_init(struct txstack_entry* self)
{
    assert(self);

    self->internal.prev = NULL;
}

void
txstack_entry_init_head(struct txstack_entry* self)
{
    assert(self);

    /* The stack head refers to itself. */
    self->internal.prev = self;
}

PICOTM_EXPORT
void
txstack_entry_uninit(struct txstack_entry* self)
{
    assert(self);
    assert(!txstack_entry_is_stacked(self));
}

void
txstack_entry_uninit_head(struct txstack_entry* self)
{
    assert(self);
    assert(txstack_entry_prev(self) == self);
}

struct txstack_entry*
txstack_entry_prev(const struct txstack_entry* self)
{
    assert(self);

    return self->internal.prev;
}

bool
txstack_entry_is_stacked(const struct txstack_entry* self)
{
    assert(self);

    return !!self->internal.prev;
}

void
txstack_entry_insert(struct txstack_entry* self, struct txstack_entry* next)
{
    assert(self);
    assert(!txstack_entry_is_stacked(self));
    assert(txstack_entry_is_stacked(next));

    struct txstack_entry* prev = txstack_entry_prev(next);

    next->internal.prev = self;
    self->internal.prev = prev;
}

void
txstack_entry_erase(struct txstack_entry* self, struct txstack_entry* next)
{
    assert(self);
    assert(txstack_entry_is_stacked(self));
    assert(txstack_entry_is_stacked(next));

    struct txstack_entry* prev = txstack_entry_prev(self);

    next->internal.prev = prev;
    self->internal.prev = NULL;
}

/*
 * Stack-entry helpers
 */

size_t
txstack_entry_distance(const struct txstack_entry* beg,
                       const struct txstack_entry* end)
{
    size_t n = 0;

    for (; beg != end; beg = txstack_entry_prev(beg)) {
        ++n;
    }

    return n;
}
