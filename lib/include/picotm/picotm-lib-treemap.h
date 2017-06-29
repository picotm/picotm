/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#pragma once

#include <stdint.h>
#include "compiler.h"

/**
 * \ingroup group_modules
 * \file
 *
 * \brief Contains `struct picotm_treemap` and helpers.
 *
 * The data stucture `struct picotm_treemap` maps keys to values on single
 * threads. Concurrent look-up by multiple transactions is *not* supported.
 * Keys can be up to 64 bit in length, values are of type `uintptr_t`, so
 * unsigned integers or pointers can be stored.
 *
 * Initialize a treemap with a call to `picotm_treemap_init()`.
 *
 * ~~~
 *      struct picotm_treemap_init treemap;
 *      picotm_treemap_init(&treemap, 13);
 * ~~~
 *
 * The second argument is the number of key bits handled per level of the
 * tree hierarchy. Ideally this number is a remainder-free divider of the
 * maximum key length. The maximum key length itself does not have ot be
 * specified. The treemap's implementation will grow the internal tree to
 * adapt to any key.
 *
 * Call `picotm_treemap_find_value()` to retrieve a key's value from the
 * treemap. If the key/value pair is not in the treemap, a creator function
 * can generate a new value as part of the lookup. If you don't supply a
 * creator function, 0 is returned for non-existing values.
 *
 * The following example returns the value for the key 0x1234 from the
 * treemap, or inserts a new value if the key/value pair is not present.
 *
 * ~~~ c
 *      // creator function
 *      uintptr_t
 *      create_value(unsigned long long key,
 *                   struct picotm_treemap* treemap,
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
 *      struct picotm_error error;
 *      int* value = (int*)picotm_treemap_find_value(treemap, 0x1234,
 *                                                   create_value,
 *                                                   &error);
 *      if (picotm_error_is_set()) {
 *          // perform error recovery
 *      }
 * ~~~
 *
 * You can iterate over all key/value pairs stored in the treemap with
 * `picotm_treemap_for_each_value()`. It invokes a call-back function for
 * each value. Values will be sorted by their keys' order. The following
 * example prints all key/value pairs to the terminal.
 *
 * ~~~ c
 *      call_value(uintptr_t value, unsigned long long key,
 *                 struct picotm_treemap* treemap, void* data,
 *                 struct picotm_error* error)
 *      {
 *          printf("key %ull is %zu\n", key, value);
 *      }
 *
 *      picotm_treemap_for_each_value(&treemap, data, call_value, &error);
 *      if (picotm_error_is_set()) {
 *          // perform error recovery
 *      }
 * ~~~
 *
 * To uninitialize a treemap, call `picotm_treemap_uninit()`. This function
 * requires a destroy function for the values. It walk over all keys in the
 * treemap and invokes the destroy function on each key's value.
 *
 * ~~~ c
 *      // destroy function
 *      void
 *      destroy_value(uintptr_t value, struct picotm_treemap* treemap)
 *      {
 *          free((int*)value);
 *      }
 *
 *      picotm_treemap_uninit(&treemap, destroy_value);
 * ~~~
 */

PICOTM_BEGIN_DECLS

struct picotm_error;

/**
 * \brief Maps keys to values.
 */
struct picotm_treemap {

    /**
     * Top-level entry; either a directory or a value.
     * \warning This is a private field. Don't access it in module code.
     */
    uintptr_t root;

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
 * Invoked by picotm's treemap to create a new value.
 * \param       key     The value's key.
 * \param       treemap The value's treemap.
 * \param[out]  error   Returns an error from the creator function.
 * \returns A pointer to the created value.
 */
typedef uintptr_t (*picotm_treemap_value_create_function)(
    unsigned long long key,
    struct picotm_treemap* treemap,
    struct picotm_error* error);

/**
 * Invoked by picotm's treemap to destroy a value.
 * \param   value   The value to destroy.
 * \param   treemap The value's treemap.
 */
typedef void (*picotm_treemap_value_destroy_function)(
    uintptr_t value, struct picotm_treemap* treemap);

/**
 * Invoked by picotm's treemap to call a value.
 * \param       value   The value.
 * \param       key     The value's key.
 * \param       treemap The value's treemap.
 * \param       data    User data.
 * \param[out]  error   Returns an error from the creator function.
 */
typedef void (*picotm_treemap_value_call_function)(
    uintptr_t value, unsigned long long key,
    struct picotm_treemap* treemap, void* data,
    struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Initializes a treemap.
 * \param   self        The treemap to initialize.
 * \param   level_nbits The number of bits per directory level.
 */
void
picotm_treemap_init(struct picotm_treemap* self,
                    unsigned long level_nbits);

PICOTM_NOTHROW
/**
 * Uninitializes a treemap.
 * \param   self            The treemap to initialize.
 * \param   value_destroy   The destroy function for values.
 */
void
picotm_treemap_uninit(struct picotm_treemap* self,
                      picotm_treemap_value_destroy_function value_destroy);

PICOTM_NOTHROW
/**
 * Retrieves the value for a key from a treemap.
 * \param       self            The treemap.
 * \param       key             The value's key.
 * \param       value_create    The creator function for values.
 * \param[out]  error           Returns an error from the look-up function.
 * \returns The key's element.
 */
uintptr_t
picotm_treemap_find_value(struct picotm_treemap* self,
                          unsigned long long key,
                          picotm_treemap_value_create_function value_create,
                          struct picotm_error* error);

PICOTM_NOTHROW
/**
 * Iterates over all values stored in a treemap.
 * \param       self        The treemap.
 * \param       data        User data.
 * \param       value_call  The call-back function for values.
 * \param[out]  error       Returns an error from the look-up function.
 */
void
picotm_treemap_for_each_value(struct picotm_treemap* self, void* data,
                              picotm_treemap_value_call_function value_call,
                              struct picotm_error* error);

PICOTM_END_DECLS
