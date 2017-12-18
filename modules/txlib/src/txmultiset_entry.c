/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "txmultiset_entry.h"
#include <assert.h>
#include <stddef.h>

PICOTM_EXPORT
void
txmultiset_entry_init(struct txmultiset_entry* self)
{
    assert(self);

    self->internal.lt = NULL;
    self->internal.ge = NULL;
    self->internal.parent = NULL;
}

void
txmultiset_entry_init_head(struct txmultiset_entry* self)
{
    assert(self);

    self->internal.lt = NULL;
    self->internal.ge = NULL;
    self->internal.parent = self;
}

PICOTM_EXPORT
void
txmultiset_entry_uninit(struct txmultiset_entry* self)
{
    assert(self);
    assert(!self->internal.lt);
    assert(!self->internal.ge);
    assert(!self->internal.parent);
}

void
txmultiset_entry_uninit_head(struct txmultiset_entry* self)
{
    assert(self);
    assert(!self->internal.lt);
    assert(!self->internal.ge);
    assert(self->internal.parent == self);
}

PICOTM_EXPORT
struct txmultiset_entry*
txmultiset_entry_next_tx(const struct txmultiset_entry* self)
{
    return txmultiset_entry_next(self);
}

PICOTM_EXPORT
struct txmultiset_entry*
txmultiset_entry_prev_tx(const struct txmultiset_entry* self)
{
    return txmultiset_entry_prev(self);
}

static struct txmultiset_entry*
bottom_most_lt(const struct txmultiset_entry* entry)
{
    while (entry->internal.lt) {
        entry = entry->internal.lt;
    }
    return (struct txmultiset_entry*)entry;
}

static struct txmultiset_entry*
bottom_most_ge(const struct txmultiset_entry* entry)
{
    while (entry->internal.ge) {
        entry = entry->internal.ge;
    }
    return (struct txmultiset_entry*)entry;
}

/**
 * \brief Returns the nearest parent that contains `entry` on its 'lt' side.
 */
static struct txmultiset_entry*
nearest_lt_parent(const struct txmultiset_entry* entry)
{
    while (entry->internal.parent->internal.ge == entry) {
        entry = entry->internal.parent;
    }
    assert(entry->internal.parent->internal.lt == entry);
    return (struct txmultiset_entry*)entry->internal.parent;
}

/**
 * \brief Returns the nearest parent that contains `entry` on its 'ge' side.
 */
static struct txmultiset_entry*
nearest_ge_parent(const struct txmultiset_entry* entry)
{
    while (entry->internal.parent->internal.lt == entry) {
        entry = entry->internal.parent;
    }
    assert(entry->internal.parent->internal.ge == entry);
    return (struct txmultiset_entry*)entry->internal.parent;
}

struct txmultiset_entry*
txmultiset_entry_begin(const struct txmultiset_entry* self)
{
    return bottom_most_lt(self);
}

struct txmultiset_entry*
txmultiset_entry_end(const struct txmultiset_entry* self)
{
    assert(self);

    return self->internal.parent;
}

struct txmultiset_entry*
txmultiset_entry_next(const struct txmultiset_entry* self)
{
    assert(self);
    assert(txmultiset_entry_is_enqueued(self));

    /* We're the parent's 'ge' entry. This means that we have to
     * return our bottom-most 'lt' entry if we're on the downward
     * move. On the upward move, we return the bottom-most 'lt'
     * entry of our 'ge' entry if it exists or the parent. */

    if (self->internal.ge) {
        return bottom_most_lt(self->internal.ge);
    } else {
        return nearest_lt_parent(self);
    }
}

struct txmultiset_entry*
txmultiset_entry_prev(const struct txmultiset_entry* self)
{
    assert(self);
    assert(txmultiset_entry_is_enqueued(self));

    if (self->internal.lt) {
        return bottom_most_ge(self->internal.lt);
    } else {
        return nearest_ge_parent(self);
    }
}

struct txmultiset_entry*
txmultiset_entry_parent(const struct txmultiset_entry* self)
{
    assert(self);
    assert(txmultiset_entry_is_enqueued(self));

    return self->internal.parent;
}

bool
txmultiset_entry_is_enqueued(const struct txmultiset_entry* self)
{
    assert(self);

    return !!self->internal.parent;
}

static void
insert_lt_entry(struct txmultiset_entry* entry, struct txmultiset_entry* tree)
{
    entry->internal.parent = tree;
    entry->internal.lt = tree->internal.lt;
    tree->internal.lt = entry;
}

static void
insert_ge_entry(struct txmultiset_entry* entry, struct txmultiset_entry* tree)
{
    entry->internal.parent = tree;
    entry->internal.ge = tree->internal.ge;
    tree->internal.ge = entry;
}

static void
insert_at(struct txmultiset_entry* entry, struct txmultiset_entry* position)
{
    entry->internal.lt = position->internal.lt;
    entry->internal.ge = position;
    entry->internal.parent = position->internal.parent;

    if (position->internal.parent->internal.lt == position) {
        position->internal.parent->internal.lt = entry;
    } else {
        assert(position->internal.parent->internal.ge == position);
        position->internal.parent->internal.ge = entry;
    }

    position->internal.parent = entry;
}

void
txmultiset_entry_insert(struct txmultiset_entry* self,
                        struct txmultiset_entry* head,
                        int (*cmp_keys)(const void* lhs, const void* rhs),
                        const void* (*key_of_entry)(struct txmultiset_entry*))
{
    assert(self);
    assert(head);
    assert(!txmultiset_entry_is_enqueued(self));

    if (!head->internal.lt) {
        insert_lt_entry(self, head);
        return;
    }

    const void* self_key = key_of_entry(self);

    struct txmultiset_entry* tree = head->internal.lt;

    do {
        int cmp = cmp_keys(self_key, key_of_entry(tree));

        if (cmp < 0) {

            if (!tree->internal.lt) {
                insert_lt_entry(self, tree);
                return;
            }

            tree = tree->internal.lt;

        } else if (cmp > 0) {

            if (!tree->internal.ge) {
                insert_ge_entry(self, tree);
                return;
            }

            tree = tree->internal.ge;

        } else {
            insert_at(self, tree);
            return;
        }
    } while (true);
}

void
txmultiset_entry_erase(struct txmultiset_entry* self)
{
    assert(self);
    assert(txmultiset_entry_is_enqueued(self));

    struct txmultiset_entry* lt = self->internal.lt;
    struct txmultiset_entry* ge = self->internal.ge;
    struct txmultiset_entry* parent = self->internal.parent;

    if (ge) {

        /* Pull 'ge' up into position of 'self' */

        ge->internal.parent = parent;

        if (parent->internal.lt == self) {
            parent->internal.lt = ge;
        } else {
            assert(parent->internal.ge == self);
            parent->internal.ge = ge;
        }

        /* Push 'lt' below 'ge' */

        if (lt) {
            struct txmultiset_entry* bm = bottom_most_lt(ge);
            lt->internal.parent = bm;
            bm->internal.lt = lt;
        }

    } else if (lt) {

        /* No 'ge', just pull up 'lt' into position of 'self' */

        lt->internal.parent = parent;

        if (parent->internal.lt == self) {
            parent->internal.lt = lt;
        } else {
            assert(parent->internal.ge == self);
            parent->internal.ge = lt;
        }

    } else {

        if (parent->internal.lt == self) {
            parent->internal.lt = NULL;
        } else {
            assert(parent->internal.ge == self);
            parent->internal.ge = NULL;
        }
    }

    /* Remove 'self' from tree */

    self->internal.lt = NULL;
    self->internal.ge = NULL;
    self->internal.parent = NULL;
}

struct txmultiset_entry*
txmultiset_entry_find(struct txmultiset_entry* self,
                      const void* key,
                      int (*cmp_keys)(const void* lhs, const void* rhs),
                      const void* (*key_of_entry)(struct txmultiset_entry*))
{
    assert(self);
    assert(key);
    assert(cmp_keys);
    assert(key_of_entry);
    assert(txmultiset_entry_is_enqueued(self));

    struct txmultiset_entry* entry = self->internal.lt;

    while (entry) {

        int cmp = cmp_keys(key, key_of_entry(entry));

        if (cmp < 0) {
            entry = entry->internal.lt;
        } else if (cmp > 0) {
            entry = entry->internal.ge;
        } else {
            return entry;
        }
    }

    return self;
}

/*
 * Multiset-entry helpers
 */

size_t
txmultiset_entry_distance(const struct txmultiset_entry* beg,
                          const struct txmultiset_entry* end)
{
    size_t n = 0;

    for (; beg != end; beg = txmultiset_entry_next(beg)) {
        ++n;
    }

    return n;
}
