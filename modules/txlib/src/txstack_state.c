/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
