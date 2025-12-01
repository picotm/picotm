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

#include "picotm/picotm-lib-shared-treemap.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"

/*
 * per-level directories
 */

struct shared_treemap_dir {
    atomic_uintptr_t entry[0];
};

static void
dir_init(struct shared_treemap_dir* self, unsigned long nentries,
         struct picotm_error* error)
{
    assert(self);

    atomic_uintptr_t* beg = picotm_arraybeg(self->entry);
    const atomic_uintptr_t* end = picotm_arrayat(self->entry, nentries);

    while (beg < end) {
        atomic_init(beg, 0);
        ++beg;
    }
}

static void
dir_uninit(struct shared_treemap_dir* self)
{ }

static size_t
sizeof_dir(unsigned long nentries)
{
    return sizeof(struct shared_treemap_dir) +
        nentries * sizeof(atomic_uintptr_t);
}

static struct shared_treemap_dir*
dir_create(unsigned long nentries, struct picotm_error* error)
{
    struct shared_treemap_dir* dir = malloc(sizeof_dir(nentries));
    if (!dir) {
        picotm_error_set_errno(error, errno);
        return nullptr;
    }

    dir_init(dir, nentries, error);
    if (picotm_error_is_set(error)) {
        goto err_dir_init;
    }

    return dir;

err_dir_init:
    free(dir);
    return nullptr;
}

static void
dir_destroy(struct shared_treemap_dir* dir)
{
    dir_uninit(dir);
    free(dir);
}

/*
 * tree
 */

static void
recursive_cleanup_dir(
    struct shared_treemap_dir* dir, unsigned long depth,
    unsigned long level_nentries, struct picotm_shared_treemap* treemap,
    picotm_shared_treemap_value_destroy_function value_destroy)
{
    assert(dir);
    assert(depth);
    assert(value_destroy);

    atomic_uintptr_t* beg = picotm_arraybeg(dir->entry);
    const atomic_uintptr_t* end = picotm_arrayat(dir->entry, level_nentries);

    --depth;

    if (depth) {

        /* recurse into directory */

        while (beg < end) {

            uintptr_t entry = atomic_load_explicit(beg, memory_order_relaxed);
            if (entry) {
                recursive_cleanup_dir((struct shared_treemap_dir*)entry,
                                      depth, level_nentries, treemap,
                                      value_destroy);
            }

            ++beg;
        }
    } else {

        /* cleanup values */

        while (beg < end) {

            uintptr_t entry = atomic_load_explicit(beg, memory_order_relaxed);
            if (entry) {
                value_destroy(entry, treemap);
            }

            ++beg;
        }
    }

    dir_destroy(dir);
}

static unsigned long
min(unsigned long lhs, unsigned long rhs)
{
    return lhs < rhs ? lhs : rhs;
}

static unsigned long
tree_depth(unsigned long key_nbits, unsigned long level_nbits)
{
    unsigned long depth = 0;

    for (;
         key_nbits;
         key_nbits -= min(key_nbits, level_nbits)) {
        ++depth;
    }

    return depth;
}

PICOTM_EXPORT
void
picotm_shared_treemap_init(struct picotm_shared_treemap* self,
                           unsigned long key_nbits,
                           unsigned long level_nbits)
{
    assert(self);

    atomic_init(&self->root, (uintptr_t)(void*)nullptr);
    self->depth = tree_depth(key_nbits, level_nbits);
    self->level_nbits = level_nbits;
}

static unsigned long
level_nentries(unsigned long level_nbits)
{
    assert(level_nbits < (sizeof(unsigned long) * CHAR_BIT));

    return 1ul << level_nbits;
}

PICOTM_EXPORT
void
picotm_shared_treemap_uninit(
    struct picotm_shared_treemap* self,
    picotm_shared_treemap_value_destroy_function value_destroy)
{
    assert(self);

    uintptr_t root = atomic_load_explicit(&self->root, memory_order_relaxed);

    if (root) {
        if (self->depth) {
            recursive_cleanup_dir((struct shared_treemap_dir*)root,
                                  self->depth,
                                  level_nentries(self->level_nbits),
                                  self, value_destroy);
        } else {
            value_destroy(root, self);
        }
    }
}

static struct shared_treemap_dir*
retrieve_dir(atomic_uintptr_t* entry_ptr, bool create_dirs,
             unsigned long level_nentries, struct picotm_error* error)
{
    uintptr_t entry = atomic_load_explicit(entry_ptr, memory_order_acquire);
    if (entry) {
        return (struct shared_treemap_dir*)entry;
    }

    if (!create_dirs) {
        return nullptr; /* don't create new directory */
    }

    /* insert missing directory */

    struct shared_treemap_dir* dir = dir_create(level_nentries, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }

    entry = (uintptr_t)dir;
    uintptr_t expected = (uintptr_t)(void*)nullptr;
    bool succ = atomic_compare_exchange_strong_explicit(entry_ptr,
                                                        &expected,
                                                        entry,
                                                        memory_order_release,
                                                        memory_order_relaxed);
    if (!succ) {
        /* A concurrent transaction already created the level's directory. */
        dir_destroy(dir);
        entry = atomic_load_explicit(entry_ptr, memory_order_acquire);
    }

    return (struct shared_treemap_dir*)entry;
}

static unsigned long
level_mask(unsigned long level_nbits)
{
    return level_nentries(level_nbits) - 1;
}

static unsigned long
entry_index(unsigned long long key, unsigned long depth,
            unsigned long level_nbits)
{
    return (key >> (depth * level_nbits)) & level_mask(level_nbits);
}

static atomic_uintptr_t*
lookup_value_entry(atomic_uintptr_t* entry_ptr, unsigned long depth,
                   unsigned long long key, bool create_dirs,
                   unsigned long level_nbits, struct picotm_error* error)
{
    /* We walk down directory tree until we reach value level. */

    while (depth) {

        --depth;

        struct shared_treemap_dir* dir  = retrieve_dir(
            entry_ptr, create_dirs, level_nentries(level_nbits), error);
        if (picotm_error_is_set(error)) {
            return nullptr;
        } else if (!dir) {
            assert(!create_dirs);
            return nullptr;
        }

        entry_ptr = dir->entry + entry_index(key, depth, level_nbits);
    }

    return entry_ptr;
}

static uintptr_t
retrieve_value(
    atomic_uintptr_t* entry_ptr,
    unsigned long long key, struct picotm_shared_treemap* treemap,
    picotm_shared_treemap_value_create_function value_create,
    picotm_shared_treemap_value_destroy_function value_destroy,
    struct picotm_error* error)
{
    uintptr_t entry = atomic_load_explicit(entry_ptr, memory_order_acquire);
    if (entry) {
        return entry;
    }

    if (!value_create) {
        return 0; /* don't create new value */
    }

    /* insert missing value */

    entry = value_create(key, treemap, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    uintptr_t expected = (uintptr_t)(void*)nullptr;
    bool succ = atomic_compare_exchange_strong_explicit(entry_ptr,
                                                        &expected,
                                                        entry,
                                                        memory_order_release,
                                                        memory_order_relaxed);
    if (!succ) {
        /* A concurrent transaction already created the value. */
        value_destroy(entry, treemap);
        entry = atomic_load_explicit(entry_ptr, memory_order_acquire);
    }

    return entry;
}

PICOTM_EXPORT
uintptr_t
picotm_shared_treemap_find_value(
    struct picotm_shared_treemap* self, unsigned long long key,
    picotm_shared_treemap_value_create_function value_create,
    picotm_shared_treemap_value_destroy_function value_destroy,
    struct picotm_error* error)
{
    assert(self);
    assert(!value_create || value_destroy);

    atomic_uintptr_t* entry_ptr = lookup_value_entry(&self->root, self->depth,
                                                     key, !!value_create,
                                                     self->level_nbits,
                                                     error);
    if (picotm_error_is_set(error)) {
        return 0;
    } if (!entry_ptr) {
        return 0;
    }

    uintptr_t entry = retrieve_value(entry_ptr, key, self, value_create,
                                     value_destroy, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return entry;
}
