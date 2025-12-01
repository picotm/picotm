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

#include "picotm/picotm-lib-treemap.h"
#if defined(HAVE_ALLOCA_H) && HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"

/*
 * treemap directory
 */

struct treemap_dir {
    uintptr_t entry[0];
};

static void
dir_init(struct treemap_dir* self, unsigned long nentries)
{
    assert(self);

    memset(self->entry, 0, sizeof(self->entry[0]) * nentries);
}

static void
dir_uninit(struct treemap_dir* self)
{
    assert(self);
}

static size_t
sizeof_dir(unsigned long nentries)
{
    return sizeof(struct treemap_dir) + nentries * sizeof(uintptr_t);

}

static struct treemap_dir*
dir_create(unsigned long nentries, struct picotm_error* error)
{
    struct treemap_dir* dir = malloc(sizeof_dir(nentries));
    if (!dir) {
        picotm_error_set_errno(error, errno);
        return nullptr;
    }

    dir_init(dir, nentries);

    return dir;
}

static void
dir_destroy(struct treemap_dir* self)
{
    dir_uninit(self);
    free(self);
}

/*
 * treemap
 */

PICOTM_EXPORT
void
picotm_treemap_init(struct picotm_treemap* self, unsigned long level_nbits)
{
    assert(self);

    self->root = (uintptr_t)(void*)nullptr;
    self->depth = 0;
    self->level_nbits = level_nbits;
}

static void
recursive_cleanup_dir(struct treemap_dir* dir, unsigned long depth,
                      unsigned long level_nentries,
                      struct picotm_treemap* treemap,
                      picotm_treemap_value_destroy_function value_destroy)
{
    assert(dir);

    uintptr_t* beg = picotm_arraybeg(dir->entry);
    const uintptr_t* end = picotm_arrayat(dir->entry, level_nentries);

    --depth;

    if (depth) {

        /* recurse into directory */

        while (beg < end) {

            uintptr_t entry = *beg;
            if (entry) {
                recursive_cleanup_dir((struct treemap_dir*)entry, depth,
                                      level_nentries, treemap,
                                      value_destroy);
            }

            ++beg;
        }
    } else {

        /* cleanup values */

        while (beg < end) {

            uintptr_t entry = *beg;
            if (entry) {
                value_destroy(entry, treemap);
            }

            ++beg;
        }
    }

    dir_destroy(dir);
}

static unsigned long
level_nentries(unsigned long level_nbits)
{
    assert(level_nbits < (sizeof(unsigned long) * CHAR_BIT));

    return 1ul << level_nbits;
}

PICOTM_EXPORT
void
picotm_treemap_uninit(struct picotm_treemap* self,
                      picotm_treemap_value_destroy_function value_destroy)
{
    assert(self);

    if (self->root) {
        if (self->depth) {
            recursive_cleanup_dir((struct treemap_dir *)self->root,
                                  self->depth,
                                  level_nentries(self->level_nbits),
                                  self,
                                  value_destroy);
        } else {
            value_destroy(self->root, self);
        }
    }
}

static unsigned long
grow_tree(uintptr_t* root, unsigned long depth, unsigned long long key_prefix,
          unsigned long level_nbits, struct picotm_error* error)
{
    /* We insert root directories until the requested key prefix
     * is supported by the tree. */

    for (; key_prefix; key_prefix >>= level_nbits) {

        struct treemap_dir* dir = dir_create(level_nentries(level_nbits),
                                             error);
        if (picotm_error_is_set(error)) {
            return depth;
        }

        dir->entry[0] = *root;
        *root = (uintptr_t)dir;
        ++depth;
    }

    return depth;
}

static struct treemap_dir*
retrieve_dir(uintptr_t* entry_ptr, bool create_dirs,
             unsigned long level_nentries, struct picotm_error* error)
{
    uintptr_t entry = *entry_ptr;
    if (entry) {
        return (struct treemap_dir*)entry;
    }

    if (!create_dirs) {
        return nullptr; /* don't create new directory */
    }

    struct treemap_dir* dir = dir_create(level_nentries, error);
    if (picotm_error_is_set(error)) {
        return nullptr;
    }

    *entry_ptr = (uintptr_t)dir;

    return dir;
}

static unsigned long
level_mask(unsigned long level_nbits)
{
    return level_nentries(level_nbits) - 1;
}

static unsigned long long
key_depth_prefix(unsigned long long key, unsigned long depth,
                 unsigned long level_nbits)
{
    return key >> (depth * level_nbits);
}

static unsigned long
entry_index(unsigned long long key, unsigned long depth,
            unsigned long level_nbits)
{
    return key_depth_prefix(key, depth, level_nbits) & level_mask(level_nbits);
}

static uintptr_t*
lookup_value_entry(uintptr_t* entry_ptr, unsigned long depth,
                   unsigned long long key, bool create_dirs,
                   unsigned long level_nbits, struct picotm_error* error)
{
    /* We walk down the directory tree until we reach
     * the value level. */

    while (depth) {

        --depth;

        struct treemap_dir* dir = retrieve_dir(entry_ptr, create_dirs,
                                               level_nentries(level_nbits),
                                               error);
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
    uintptr_t* entry_ptr,
    unsigned long long key, struct picotm_treemap* treemap,
    picotm_treemap_value_create_function value_create,
    struct picotm_error* error)
{
    uintptr_t entry = *entry_ptr;
    if (entry) {
        return entry;
    }

    if (!value_create) {
        return 0; /* don't create new value */
    }

    entry = value_create(key, treemap, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    *entry_ptr = entry;

    return entry;
}

PICOTM_EXPORT
uintptr_t
picotm_treemap_find_value(struct picotm_treemap* self,
                          unsigned long long key,
                          picotm_treemap_value_create_function value_create,
                          struct picotm_error* error)
{
    assert(self);

    unsigned long long key_prefix = key_depth_prefix(key,
                                                     self->depth,
                                                     self->level_nbits);
    if (key_prefix) {
        self->depth = grow_tree(&self->root, self->depth, key_prefix,
                                self->level_nbits, error);
        if (picotm_error_is_set(error)) {
            return 0;
        }
    }

    uintptr_t* entry_ptr = lookup_value_entry(&self->root, self->depth,
                                              key, !!(value_create),
                                              self->level_nbits, error);
    if (picotm_error_is_set(error)) {
        return 0;
    } else if (!entry_ptr) {
        return 0;
    }

    uintptr_t value = retrieve_value(entry_ptr, key, self,
                                     value_create, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return value;
}

struct dir_stack {
    unsigned long long key;
    uintptr_t          entry;
    unsigned long      depth;
    unsigned long      i;
};

static void
walk_values(struct dir_stack* stack, unsigned long depth,
            unsigned long long level_nbits,
            struct picotm_treemap* treemap, void* data,
            picotm_treemap_value_call_function value_call,
            struct picotm_error* error)
{
    /* recurse through treemap and process values in order */

    while (depth) {

        struct dir_stack* top = stack + depth - 1;

        if (top->depth) {

            /* recurse into directory */

            const struct treemap_dir* dir = (struct treemap_dir*)top->entry;
            assert(dir);

            while (top->i < level_nentries(level_nbits)) {
                if (dir->entry[top->i]) {
                    break; /* found next valid directory entry */
                }
                ++top->i;
            }

            if (top->i < level_nentries(level_nbits)) {

                /* prepare stack for sub-directory */
                stack[depth].key = (top->key << level_nbits) + top->i;
                stack[depth].entry = dir->entry[top->i];
                stack[depth].depth = top->depth - 1;
                stack[depth].i = 0;
                ++top->i;
                ++depth;

            } else {
                /* end of directory reached; go up one level */
                --depth;
            }
        } else {

            /* invoke call-back function for value */
            value_call(top->entry, top->key, treemap, data, error);
            if (picotm_error_is_set(error)) {
                return;
            }

            /* value processed; go up one level */
            --depth;
        }
    }
}

static size_t
sizeof_dir_stack(unsigned long treemap_depth)
{
    return (treemap_depth + 1) * sizeof(struct dir_stack);
}

PICOTM_EXPORT
void
picotm_treemap_for_each_value(struct picotm_treemap* self, void* data,
                              picotm_treemap_value_call_function value_call,
                              struct picotm_error* error)
{
    assert(self);
    assert(value_call);

    /* allocate enough stack to hold n directories plus 1 value */

#if !defined(HAVE_ALLOCA) || !HAVE_ALLOCA
    struct dir_stack* stack = malloc(sizeof_dir_stack(self->depth));
    if (!stack) {
        picotm_error_set_errno(error, errno);
        return;
    }
#else
    struct dir_stack* stack = alloca(sizeof_dir_stack(self->depth));
#endif

    size_t depth = 0; /* stack depth; *not* tree depth! */

    if (self->root) {
        stack[depth].key = 0;
        stack[depth].entry = self->root;
        stack[depth].depth = self->depth;
        stack[depth].i = 0;
        ++depth;
    }

    walk_values(stack, depth, self->level_nbits, self, data, value_call,
                error);
    if (picotm_error_is_set(error)) {
        goto err_walk_values;
    }

#if !defined(HAVE_ALLOCA) || !HAVE_ALLOCA
    free(stack);
#endif

    return;

err_walk_values:
#if !defined(HAVE_ALLOCA) || !HAVE_ALLOCA
    free(stack);
#endif
    return;
}
