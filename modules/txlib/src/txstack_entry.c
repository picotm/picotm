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
