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

#include "txqueue_state.h"
#include <assert.h>
#include "txqueue_entry.h"

PICOTM_EXPORT
void
txqueue_state_init(struct txqueue_state* self)
{
    assert(self);

    txqueue_entry_init_head(&self->internal.head);
    picotm_rwlock_init(&self->internal.lock);
}

PICOTM_EXPORT
void
txqueue_state_uninit(struct txqueue_state* self)
{
    assert(self);
    assert(txqueue_state_is_empty(self));

    txqueue_entry_uninit_head(&self->internal.head);
    picotm_rwlock_uninit(&self->internal.lock);
}

PICOTM_EXPORT
void
txqueue_state_clear_and_uninit_entries(struct txqueue_state* self,
                                       void (*uninit)(struct txqueue_entry*,
                                                      void*),
                                       void* data)
{
    assert(self);
    assert(uninit);

    struct txqueue_entry* beg = txqueue_state_begin(self);
    struct txqueue_entry* end = txqueue_state_end(self);

    for (; beg != end; beg = txqueue_state_begin(self)) {
        txqueue_entry_erase(beg);
        uninit(beg, data);
    }
}

bool
txqueue_state_is_empty(struct txqueue_state* self)
{
    return txqueue_state_begin(self) == txqueue_state_end(self);
}

size_t
txqueue_state_size(struct txqueue_state* self)
{
    return txqueue_entry_distance(txqueue_state_begin(self),
                                  txqueue_state_end(self));
}

struct txqueue_entry*
txqueue_state_begin(struct txqueue_state* self)
{
    assert(self);

    return txqueue_entry_next(&self->internal.head);
}

struct txqueue_entry*
txqueue_state_end(struct txqueue_state* self)
{
    assert(self);

    return &self->internal.head;
}

struct txqueue_entry*
txqueue_state_front(struct txqueue_state* self)
{
    return txqueue_state_begin(self);
}

struct txqueue_entry*
txqueue_state_back(struct txqueue_state* self)
{
    return txqueue_entry_prev(txqueue_state_end(self));
}

void
txqueue_state_push_front(struct txqueue_state* self,
                         struct txqueue_entry* entry)
{
    assert(!txqueue_entry_is_enqueued(entry));

    txqueue_entry_insert(entry, txqueue_state_begin(self));
}

void
txqueue_state_push_back(struct txqueue_state* self,
                        struct txqueue_entry* entry)
{
    assert(!txqueue_entry_is_enqueued(entry));

    txqueue_entry_insert(entry, txqueue_state_end(self));
}

void
txqueue_state_pop_front(struct txqueue_state* self)
{
    txqueue_entry_erase(txqueue_state_front(self));
}
