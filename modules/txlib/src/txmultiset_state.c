/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
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
