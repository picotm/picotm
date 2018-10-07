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
