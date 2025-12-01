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
