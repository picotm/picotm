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

#include "txlist_entry.h"
#include <assert.h>
#include <stddef.h>

PICOTM_EXPORT
void
txlist_entry_init(struct txlist_entry* self)
{
    assert(self);

    self->internal.next = NULL;
    self->internal.prev = NULL;
}

void
txlist_entry_init_head(struct txlist_entry* self)
{
    assert(self);

    self->internal.next = self;
    self->internal.prev = self;
}

PICOTM_EXPORT
void
txlist_entry_uninit(struct txlist_entry* self)
{
    assert(self);
    assert(!self->internal.next);
    assert(!self->internal.prev);
}

void
txlist_entry_uninit_head(struct txlist_entry* self)
{
    assert(self);
    assert(self->internal.next == self);
    assert(self->internal.prev == self);
}

PICOTM_EXPORT
struct txlist_entry*
txlist_entry_next_tx(const struct txlist_entry* self)
{
    return txlist_entry_next(self);
}

PICOTM_EXPORT
struct txlist_entry*
txlist_entry_prev_tx(const struct txlist_entry* self)
{
    return txlist_entry_prev(self);
}

struct txlist_entry*
txlist_entry_next(const struct txlist_entry* self)
{
    assert(self);

    struct txlist_entry* next = self->internal.next;
    assert(next);

    return next;
}

struct txlist_entry*
txlist_entry_prev(const struct txlist_entry* self)
{
    assert(self);

    struct txlist_entry* prev = self->internal.prev;
    assert(prev);

    return prev;
}

bool
txlist_entry_is_enqueued(const struct txlist_entry* self)
{
    assert(self);
    assert(!self->internal.prev == !self->internal.next);

    return !!self->internal.next;
}

void
txlist_entry_insert(struct txlist_entry* self, struct txlist_entry* next)
{
    assert(self);
    assert(next);
    assert(!txlist_entry_is_enqueued(self));
    assert(txlist_entry_is_enqueued(next));

    self->internal.next = next;
    self->internal.prev = next->internal.prev;

    next->internal.prev->internal.next = self;
    next->internal.prev = self;
}

void
txlist_entry_erase(struct txlist_entry* self)
{
    assert(self);
    assert(txlist_entry_is_enqueued(self));

    self->internal.next->internal.prev = self->internal.prev;
    self->internal.prev->internal.next = self->internal.next;

    self->internal.next = NULL;
    self->internal.prev = NULL;
}

/*
 * List-entry helpers
 */

size_t
txlist_entry_distance(const struct txlist_entry* beg,
                      const struct txlist_entry* end)
{
    size_t n = 0;

    for (; beg != end; beg = txlist_entry_next(beg)) {
        ++n;
    }

    return n;
}
