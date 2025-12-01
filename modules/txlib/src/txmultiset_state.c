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

#include "txmultiset_state.h"
#include <assert.h>
#include <stddef.h>
#include "txmultiset_entry.h"

PICOTM_EXPORT
void
txmultiset_state_init(struct txmultiset_state* self,
                      txmultiset_key_function key,
                      txmultiset_compare_function compare)
{
    assert(self);

    txmultiset_entry_init_head(&self->internal.head);
    picotm_rwlock_init(&self->internal.lock);
    self->internal.key = key;
    self->internal.compare = compare;
}

PICOTM_EXPORT
void
txmultiset_state_uninit(struct txmultiset_state* self)
{
    assert(self);

    picotm_rwlock_uninit(&self->internal.lock);
    txmultiset_entry_uninit_head(&self->internal.head);
}

PICOTM_EXPORT
void
txmultiset_state_clear_and_uninit_entries(struct txmultiset_state* self,
                                          void (*uninit)(
                                              struct txmultiset_entry*,
                                              void*),
                                          void* data)
{
    assert(self);
    assert(uninit);

    struct txmultiset_entry* beg = txmultiset_state_begin(self);
    struct txmultiset_entry* end = txmultiset_state_end(self);

    for (; beg != end; beg = txmultiset_state_begin(self)) {
        txmultiset_state_erase(self, beg);
        uninit(beg, data);
    }
}

struct txmultiset_entry*
txmultiset_state_begin(const struct txmultiset_state* self)
{
    assert(self);

    return txmultiset_entry_begin(&self->internal.head);
}

struct txmultiset_entry*
txmultiset_state_end(const struct txmultiset_state* self)
{
    assert(self);

    return txmultiset_entry_end(&self->internal.head);
}

size_t
txmultiset_state_size(const struct txmultiset_state* self)
{
    return txmultiset_entry_distance(txmultiset_state_begin(self),
                                     txmultiset_state_end(self));
}

bool
txmultiset_state_is_empty(const struct txmultiset_state* self)
{
    return txmultiset_state_begin(self) ==
           txmultiset_state_end(self);
}

struct txmultiset_entry*
txmultiset_state_front(struct txmultiset_state* self)
{
    return txmultiset_state_begin(self);
}

struct txmultiset_entry*
txmultiset_state_back(struct txmultiset_state* self)
{
    return txmultiset_entry_prev(txmultiset_state_end(self));
}

void
txmultiset_state_insert(struct txmultiset_state* self,
                        struct txmultiset_entry* entry)
{
    assert(self);

    txmultiset_entry_insert(entry, &self->internal.head,
                            self->internal.compare,
                            self->internal.key);
}

void
txmultiset_state_erase(struct txmultiset_state* self, struct txmultiset_entry* entry)
{
    assert(entry != txmultiset_state_end(self));

    txmultiset_entry_erase(entry);
}

struct txmultiset_entry*
txmultiset_state_find(struct txmultiset_state* self, const void* key)
{
    assert(self);

    return txmultiset_entry_find(&self->internal.head, key,
                                 self->internal.compare,
                                 self->internal.key);
}

struct txmultiset_entry*
txmultiset_state_lower_bound(struct txmultiset_state* self, const void* key)
{
    assert(self);

    struct txmultiset_entry* end = txmultiset_state_end(self);

    /* find an entry with the given key */

    struct txmultiset_entry* entry = txmultiset_state_find(self, key);
    if (entry == end) {
        return entry;
    }

    /* iterate to the first entry with the key */

    struct txmultiset_entry* beg = txmultiset_state_begin(self);
    struct txmultiset_entry* pos = entry;

    while (pos != beg) {

        pos = txmultiset_entry_prev(pos);

        int cmp = self->internal.compare(key, self->internal.key(pos));
        if (cmp) {
            return pos;
        }

        entry = pos;
    }

    return entry;
}

struct txmultiset_entry*
txmultiset_state_upper_bound(struct txmultiset_state* self, const void* key)
{
    assert(self);

    struct txmultiset_entry* end = txmultiset_state_end(self);

    /* find an entry with the given key */

    struct txmultiset_entry* entry = txmultiset_state_find(self, key);
    if (entry == end) {
        return entry;
    }

    /* iterate to the last entry with the key */

    struct txmultiset_entry* pos = txmultiset_entry_next(entry);

    for (; pos != end; pos = txmultiset_entry_next(pos)) {

        int cmp = self->internal.compare(key, self->internal.key(pos));
        if (cmp) {
            return pos;
        }

        entry = pos;
    }

    return entry;
}

size_t
txmultiset_state_count(struct txmultiset_state* self, const void* key)
{
    assert(self);

    struct txmultiset_entry* end = txmultiset_state_end(self);

    /* find an entry with the given key */

    struct txmultiset_entry* entry = txmultiset_state_find(self, key);
    if (entry == end) {
        return 0;
    }

    /* iterate to the first entry with the key */

    struct txmultiset_entry* beg = txmultiset_state_begin(self);
    struct txmultiset_entry* pos = entry;
    struct txmultiset_entry* lower = entry;

    while (pos != beg) {

        pos = txmultiset_entry_prev(pos);

        int cmp = self->internal.compare(key, self->internal.key(pos));
        if (cmp) {
            break;
        }

        lower = pos;
    }

    /* iterate to the last entry with the key */

    struct txmultiset_entry* upper = entry;

    for (; upper != end; upper = txmultiset_entry_next(upper)) {

        int cmp = self->internal.compare(key, self->internal.key(upper));
        if (cmp) {
            break;
        }
    }

    return txmultiset_entry_distance(lower, upper);
}
