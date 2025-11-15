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
