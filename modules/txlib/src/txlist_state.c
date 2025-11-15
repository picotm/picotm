/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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
