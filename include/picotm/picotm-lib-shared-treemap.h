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

#pragma once

#include <stdatomic.h>
#include <stdint.h>
#include "compiler.h"

/**
 * \ingroup group_lib
 * \file
 * \brief Contains `struct picotm_shared_treemap` and helper functions.
 *
 * The data stucture `struct picotm_shared_treemap` maps keys to values.
 * Concurrent look-up by multiple transactions is supported. Keys can be up
 * to 64 bit in length, values are of type `uintptr_t`, so unsigned integers
 * or pointers can be stored.
 *
 * Initialize a shared treemap with a call to `picotm_shared_treemap_init()`.
 *
 * ~~~
 *      struct picotm_shared_treemap_init treemap;
 *      picotm_shared_treemap_init(&treemap, 52, 13);
 * ~~~
 *
 * The second parameter is the maximum number of bits in a key. The
 * initializer function sets up the shared treemap instance to support keys
 * up to this length. Using larger keys is undefined. The third argument is
 * the number of key bits handled per level of the internal tree hierarchy.
 * Ideally this number is a remainder-free divider of the key length.
 *
 * Call `picotm_shared_treemap_find_value()` to retrieve a key's value from
 * the shared treemap. If the key/value pair is not in the shared treemap, a
 * creator function can generate a new value as part of the lookup. If you
 * don't provide a creator function, 0 is returned for non-existing values.
 *
 * The following example returns the value for the key 0x1234 from the
 * shared treemap, or inserts a new value if the key/value pair is not
 * present.
 *
 * ~~~ c
 *      // creator function
 *      uintptr_t
 *      create_value(unsigned long long key,
 *                   struct picotm_shared_treemap* treemap,
 *                   struct picotm_error* error)
 *      {
 *          int* value = malloc(sizeof(*value));
 *          if (!value) {
 *              picotm_set_error_code(error, PICOTM_OUT_OF_MEMORY);
 *              return 0;
 *          }
 *          *value = 0;
 *          return (uintptr_t)value;
 *      }
 *
 *      // destroy function
 *      void
 *      destroy_value(uintptr_t value, struct picotm_shared_treemap* treemap)
 *      {
 *          free((int*)value);
 *      }
 *
 *      struct picotm_error error;
 *      int* value = (int*)picotm_shared_treemap_find_value(treemap, 0x1234,
 *                                                          create_value,
 *                                                          destroy_value,
 *                                                          &error);
 *      if (picotm_error_is_set()) {
 *          // perform error recovery
 *      }
 * ~~~
 *
 * To uninitialize a shared treemap, call `picotm_shared_treemap_uninit()`.
 * This function requires a destroy function for the values. It walk over all
 * keys in the shared treemap and invokes the destroy function on each key's
 * value.
 *
 * ~~~ c
 *      picotm_shared_treemap_uninit(&treemap, destroy_value);
 * ~~~
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \ingroup group_lib
 * \brief Maps keys to values.
 */
struct picotm_shared_treemap {

    /**
     * Top-level entry; either a directory or a value.
     * \warning This is a private field. Don't access it in module code.
     */
    atomic_uintptr_t root;

    /**
     * Tree depth is the number of directories in the hierarchy.
     * \warning This is a private field. Don't access it in module code.
     */
    unsigned long depth;

    /**
     * The number of entries at each level of the tree hierarchy.
     * \warning This is a private field. Don't access it in module code.
     */
    unsigned long level_nbits;
};

/**
 * \ingroup group_lib
 * Invoked by picotm's shared treemap to create a new shared value.
 * \param       key     The value's key.
 * \param       treemap The value's shared treemap.
 * \param[out]  error   Returns an error from the creator function.
 * \returns A pointer to the created value.
 */
typedef uintptr_t (*picotm_shared_treemap_value_create_function)(
    unsigned long long key,
    struct picotm_shared_treemap* treemap,
    struct picotm_error* error);

/**
 * \ingroup group_lib
 * Invoked by picotm's shared treemap to destroy a value.
 * \param   value   The value to destroy.
 * \param   treemap The value's shared treemap.
 */
typedef void (*picotm_shared_treemap_value_destroy_function)(
    uintptr_t value, struct picotm_shared_treemap* treemap);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Initializes a shared treemap.
 * \param   self        The shared treemap to initialize.
 * \param   key_nbits   The maximum number of bits per key.
 * \param   level_nbits The number of bits per directory level.
 */
void
picotm_shared_treemap_init(struct picotm_shared_treemap* self,
                           unsigned long key_nbits,
                           unsigned long level_nbits);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Uninitializes a shared treemap.
 * \param   self            The shared treemap to initialize.
 * \param   value_destroy   The destroy function for shared values.
 */
void
picotm_shared_treemap_uninit(
    struct picotm_shared_treemap* self,
    picotm_shared_treemap_value_destroy_function value_destroy);

PICOTM_NOTHROW
/**
 * \ingroup group_lib
 * Retrieves the value for a key from a shared treemap.
 * \param       self            The shared treemap.
 * \param       key             The value's key.
 * \param       value_create    The creator function for shared values.
 * \param       value_destroy   The destroy function for shared values.
 * \param[out]  error           Returns an error from the look-up function.
 * \returns The key's element.
 */
uintptr_t
picotm_shared_treemap_find_value(
    struct picotm_shared_treemap* self, unsigned long long key,
    picotm_shared_treemap_value_create_function value_create,
    picotm_shared_treemap_value_destroy_function value_destroy,
    struct picotm_error* error);

PICOTM_END_DECLS
