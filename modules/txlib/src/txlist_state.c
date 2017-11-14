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

#include "txlist_state.h"
#include <assert.h>
#include <stddef.h>
#include "txlist_entry.h"

PICOTM_EXPORT
void
txlist_state_init(struct txlist_state* self)
{
    assert(self);

    /* The list head is a special, empty entry that refers to itself. */
    txlist_entry_init(&self->internal.head);
    self->internal.head.internal.next = &self->internal.head;
    self->internal.head.internal.prev = &self->internal.head;

    picotm_rwlock_init(&self->internal.lock);
}

PICOTM_EXPORT
void
txlist_state_uninit(struct txlist_state* self)
{
    assert(self);

    picotm_rwlock_uninit(&self->internal.lock);

    self->internal.head.internal.next = NULL;
    self->internal.head.internal.prev = NULL;
    txlist_entry_uninit(&self->internal.head);
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

static size_t
list_entry_distance(const struct txlist_entry* beg,
                    const struct txlist_entry* end)
{
    size_t n = 0;

    for (; beg != end; beg = txlist_entry_next(beg)) {
        ++n;
    }

    return n;
}

size_t
txlist_state_size(const struct txlist_state* self)
{
    return list_entry_distance(txlist_state_begin(self),
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

static void
insert_entry(struct txlist_entry* entry,
             struct txlist_entry* position)
{
    assert(entry);
    assert(position);
    assert(!txlist_entry_is_enqueued(entry));
    assert(txlist_entry_is_enqueued(position));

    entry->internal.next = position;
    entry->internal.prev = position->internal.prev;

    position->internal.prev->internal.next = entry;
    position->internal.prev = entry;
}

static void
erase_entry(struct txlist_entry* entry)
{
    assert(entry);
    assert(txlist_entry_is_enqueued(entry));

    entry->internal.next->internal.prev = entry->internal.prev;
    entry->internal.prev->internal.next = entry->internal.next;

    entry->internal.next = NULL;
    entry->internal.prev = NULL;
}

void
txlist_state_insert(struct txlist_state* self, struct txlist_entry* entry,
                    struct txlist_entry* position)
{
    insert_entry(entry, position);
}

void
txlist_state_erase(struct txlist_state* self, struct txlist_entry* entry)
{
    assert(entry != txlist_state_end(self));

    erase_entry(entry);
}
