/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#pragma once

#include "picotm/config/picotm-txlib-config.h"
#include "picotm/compiler.h"
#include <stdbool.h>
#include <stddef.h>
#include "picotm-txmultiset-state.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txmultiset
 * \file
 * \brief Provides transactional multisets
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes an entry of a transactional multiset from within a
 *        transaction.
 * \param self The multiset entry to initialize.
 * \attention This function expects the entry's memory to be owned by
 *            the calling transaction. Shared-memory locations have to
 *            be read/write privatized first.
 */
void
txmultiset_entry_init_tm(struct txmultiset_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up an entry of a transactional multiset from within a
 *        transaction.
 * \param self The multiset entry to clean up.
 * \attention This function expects the entry's memory to be owned by
 *            the calling transaction. Shared-memory locations have to
 *            be read/write privatized first.
 */
void
txmultiset_entry_uninit_tm(struct txmultiset_entry* self);

/**
 * \ingroup group_txlib
 * \struct txmultiset
 * \brief A handle for operating on transaction-safe multisets.
 */
struct txmultiset;

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Creates a transactional multiset for a multiset state.
 * \param multiset_state The multiset state.
 * \returns A transactional multiset for the multiset state.
 */
struct txmultiset*
txmultiset_of_state_tx(struct txmultiset_state* multiset_state);

/*
 * Entries
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the first entry of a transactional multiset.
 * \param self The transactional multiset.
 * \returns The transactional multiset's first entry.
 */
struct txmultiset_entry*
txmultiset_begin_tx(struct txmultiset* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the terminator of a transactional multiset.
 * \param self The transactional multiset.
 * \returns The transactional multiset's terminator entry.
 */
struct txmultiset_entry*
txmultiset_end_tx(struct txmultiset* self);

/*
 * Capacity
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Tests a transactional multiset for emptiness.
 * \param self The transactional multiset.
 * \returns True if the multiset is empty, false otherwise.
 */
bool
txmultiset_empty_tx(struct txmultiset* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the number of entries in a transactional multiset.
 * \param self The transactional multiset.
 * \returns The number of entries in the transactional multiset.
 */
size_t
txmultiset_size_tx(struct txmultiset* self);

/*
 * Modfiers
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes all entries from a transactional multiset.
 * \param self The transactional multiset.
 */
void
txmultiset_clear_tx(struct txmultiset* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes an entry from a transactional multiset.
 * \param self The transactional multiset.
 * \param entry The multiset entry to remove.
 */
void
txmultiset_erase_tx(struct txmultiset* self, struct txmultiset_entry* entry);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Inserts an entry into a transactional multiset.
 * \param self The transactional multiset.
 * \param entry The multiset entry to insert.
 */
void
txmultiset_insert_tx(struct txmultiset* self, struct txmultiset_entry* entry);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Finds an entry with a specific key in a transactional multiset.
 * \param self The transactional multiset.
 * \param key The multiset entry's key.
 * \returns An entry with the given key on success, or the terminator entry
 *          otherwise.
 */
struct txmultiset_entry*
txmultiset_find_tx(struct txmultiset* self, const void* key);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns a transactional multiset's the first entry with a specific
 *        key.
 * \param self The transactional multiset.
 * \param key The multiset entry's key.
 * \returns The first entry with the given key on success, or the terminator
 *          entry otherwise.
 */
struct txmultiset_entry*
txmultiset_lower_bound_tx(struct txmultiset* self, const void* key);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns a transactional multiset's first entry with a key larger
 *        than a specific key.
 * \param self The transactional multiset.
 * \param key The multiset entry's key.
 * \returns The first entry with a key that is larger than the given key
 *          on success, or the terminator entry otherwise.
 */
struct txmultiset_entry*
txmultiset_upper_bound_tx(struct txmultiset* self, const void* key);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the number of entries with a specfic key in a transactional
 *        multiset.
 * \param self The transactional multiset.
 * \param key The multiset entry's key.
 * \returns The number of entries with the given key.
 */
size_t
txmultiset_count_tx(struct txmultiset* self, const void* key);

PICOTM_END_DECLS

/**
 * \defgroup group_txlib_txmultiset Transactional Multisets
 *
 * \brief The transactional multiset provides a transaction-safe
 *        implementation of a multiset. Multisets are sorted sets
 *        of elements. Duplicate entries are supported.
 *
 * The `struct txmultiset` data structure represents a transactional
 * multiset (i.e., a set that can hold the same value multiple times). To
 * create a multiset, we first need multiset entries. These are represented
 * by `struct txmultiset_entry`. We can add an instance of this data
 * structure to any data value to turn it into a multiset entry. Here's an
 * example for multisets of values of type `unsigned long`.
 *
 * ~~~ c
 *      struct ulong_item {
 *          struct txmultiset_entry multiset_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      void
 *      ulong_item_init(struct ulong_item* item, unsigned long value)
 *      {
 *          txmultiset_entry_init(&item->multiset_entry);
 *          item->value = value;
 *      }
 *
 *      struct ulong_item item;
 *
 *      ulong_item_init(&item, 0);
 * ~~~
 *
 * This code initializes the multiset entry using `txmultiset_entry_init()`. The
 * macro `TXMULTISET_ENTRY_INITIALIZER` initializes static or stack-allocated
 * multiset entries. The example below illustrates this.
 *
 * ~~~ c
 *      struct ulong_item {
 *          struct txmultiset_entry multiset_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      #define ULONG_ITEM_INITIALIZER(_value)  \
 *      {                                       \
 *          TXMULTISET_ENTRY_INITIALIZER,       \
 *          (_value)                            \
 *      }
 *
 *      struct ulong_item item = ULONG_ITEM_INITIALIZER(0);
 * ~~~
 *
 * When both, macro and function initialization, is possible, the macro
 * form is prefered. Multiset entries are uninitialized with
 * `txmultiset_entry_uninit()`.
 *
 * ~~~ c
 *      void
 *      ulong_item_uninit(struct ulong_item* item)
 *      {
 *          txmultiset_entry_uninit(&item->multiset_entry);
 *      }
 *
 *      ulong_item_uninit(&item);
 * ~~~
 *
 * To store the multiset entries, we need a non-transactional multiset state,
 * which represents the shared state of a multiset. It's implemented by
 * `struct txmultiset_state`. We can define and initialize a multiset state
 * as illustrated in the example below.
 *
 * ~~~ c
 *      struct ulong_item*
 *      ulong_item_of_entry(struct txmultiset_entry* entry)
 *      {
 *          return picotm_containerof(entry, struct ulong_item, multiset_entry);
 *      }
 *
 *      const void*
 *      ulong_item_key(struct ulong_item* item)
 *      {
 *          return &item->value;
 *      }
 *
 *      int
 *      ulong_compare(const unsigned long* lhs, const unsigned long* rhs)
 *      {
 *          return (*rhs < *lhs) - (*lhs < *rhs);
 *      }
 *
 *      const void*
 *      key_cb(struct txmultiset_entry* entry)
 *      {
 *          return ulong_item_key(ulong_item_of_entry(entry));
 *      }
 *
 *      int
 *      compare_cb(const void* lhs, const void* rhs)
 *      {
 *          return ulong_compare(lhs, rhs);;
 *      }
 *
 *      struct txmultiset_state multiset_state;
 *
 *      txmultiset_state_init(&multiset_state, key_cb, compare_cb);
 * ~~~
 *
 * The initializer function takes the multiset state and two additional
 * call-back functions, which are required for sorting the multiset's
 * entries. The `key_cb()` call-back function returns a pointer to the key
 * for a given entry. In the example above, it's the value itself. The
 * `compare_cb()` call-back function compares two keys. It returns a value
 * less than, equal to, or greater than zero if the left-hand-side value
 * is less than, equal to, or greater than the right-hand-side value.
 * Entries in a multiset are sorted by their key in ascending order. These
 * call-back functions provide the key and the compare operation.
 *
 * The code uses the initializer function `txmultiset_state_init()`. For
 * static or stack-allocated multiset states, there's the initializer macro
 * `TXMULTISET_STATE_INITIALIZER()`.
 *
 * ~~~ c
 *      struct txmultiset_state multiset_state = TXMULTISET_STATE_INITIALIZER(multiset_state, key_cb, compare_cb);
 * ~~~
 *
 * When both forms are possible, the initializer macro is prefered.
 *
 * Multiset-state clean-up is performed by `txmultiset_state_uninit()`. The
 * multiset state may not contain entries when the clean-up happens. This
 * means that entries have to be removed from within a transaction.
 *
 * For many uses this requirement just imposes unnecessary overhead. A call
 * to `txmultiset_state_clear_and_uninit_entries()` provies a non-transactional
 * way of clearing the multiset from its entries. It erases each element from
 * the multiset and calls a clean-up function on it. The example below shows
 * the clean-up code for a multiset of `struct ulong_item` entries.
 *
 * ~~~ c
 *      void
 *      ulong_item_uninit_cb(struct txmultiset_entry* entry, void* data)
 *      {
 *          ulong_item_uninit(ulong_item_of_entry(entry));
 *
 *          // call free() if item was malloc()'ed
 *      }
 *
 *      txmultiset_state_clear_and_uninit_entries(&multiset_state, ulong_item_uninit_cb, NULL);
 *      txmultiset_state_uninit(&multiset_state);
 * ~~~
 *
 * At this point we have a multiset state and multiset entries to add to the state.
 * So far all code was non-transactional. Actual multiset access and manipulation
 * is performed by transactional code.
 *
 * To perform multiset operations within a transaction, we first need a multiset
 * data structure for our transaction. It's represented by `struct txmultiset`.
 * A call to `txmultiset_of_state_tx()` returns an instance.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * Calling `txmultiset_of_state_tx()` multiple times for the same multiset
 * state *within the same transaction* returns the same multiset. Multisets
 * are undefined after their transaction committed or aborted, or within
 * other, concurrent transactions.
 *
 * With the multiset, we can now insert entries into the multiset state using
 * `txmultiset_insert_tx()`. The example below illustrates this.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          txmultiset_insert_tx(multiset, &item->multiset_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * After this transaction committed, the ulong data item `item` will be
 * the in `multiset_state`. If the transactions has to abort after
 * the call to `txmultiset_insert_tx()`, the transaction framework will
 * automatically remove the entry during the rollback; thus restoring the
 * original state.
 *
 * The inserted entry will be sorted in ascending order into the multiset,
 * using the key the compare call-back functions. The insert function compares
 * the key of the new entry to the keys of existing entries to determines the
 * new entry's position. The set is implemented as a tree, so inserting
 * requires a compare operation with only a small fraction of existing items.
 *
 * To remove an entry from the multiset, we can call `txmultiset_erase_tx()`, as
 * illustated in the example below.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          // The multiset state already contains the entry.
 *          txmultiset_erase_tx(multiset, &item->multiset_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * Removing an entry keeps the entries sorted. The multiset's implementation
 * re-arranges the entries automatically with very little overhead. As usual,
 * all errors are detected and handled by the transaction framework. The
 * benefits of transactional code show when we move entries between multisets.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* src_multiset = txmultiset_of_state_tx(&src_multiset_state);
 *          struct txmultiset* dst_multiset = txmultiset_of_state_tx(&dst_multiset_state);
 *
 *          // The multiset state already contains the entry.
 *          txmultiset_erase_tx(src_multiset, &item->multiset_entry);
 *
 *          txmultiset_insert_tx(dst_multiset, &item->multiset_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * In this example, we removed an entry from a source multiset and inserted it
 * into a destination multiset. The transaction framework automatically isolates
 * these operations from concurrent transactions until the transaction commits.
 * So concurrent transactions see the entry in *either* the source multiset
 * *or* the destination multiset, but never in both. If the transaction has to
 * roll back after the insert operation, the transaction framework
 * automatically removes the multiset entry from the destination multiset and
 * returns it to its old position in the source multiset.
 *
 * A call to `txmultiset_size_tx()` returns the number of multiset entries, a
 * call to `txmultiset_empty_tx()` returns `true` if a multiset is empty. The
 * former function might have linear complexity, the later function always has
 * constant complexity. It's therefore better to use `txmultiset_empty_tx()`
 * if it's only relevant whether there are entries.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          bool is_empty = txmultiset_empty_tx(multiset);
 *
 *          size_t size = txmultiset_size_tx(multiset);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * We can iterate over the entries of a multiset. The first entry of the
 * multiset is returned by `txmultiset_begin_tx()`. The terminator entry
 * *after* the final entry is returned by `txmultiset_end_tx()`. The
 * terminator entry is not a real entry and should not be dereferenced. Calls
 * to `txmultiset_entry_next_tx()` and `txmultiset_entry_prev_tx()` return an
 * entry's successor or predecessor. Here's an example that iterates over a
 * multiset of values of type `unsigned long` and sums up the individual
 * values.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      unsigned long g_sum; // global variable containg sum of multiset entries
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          unsigned long sum = 0;
 *
 *          struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
 *          struct txmultiset_entry* end = txmultiset_end_tx(multiset);
 *
 *          while (beg != end) {
 *
 *              struct ulong_item* item = ulong_item_of_entry(beg);
 *
 *              sum += item->value;
 *
 *              beg = txmultiset_entry_next_tx(beg);
 *          }
 *
 *          // more transactional code
 *
 *          // Picotm's modules collaborate! The value stored in
 *          // `sum` is exported from the transaction state using
 *          // `store_ulong_tx()` from the Transactional Memory
 *          // module.
 *
 *          store_ulong_tx(&g_sum, sum);
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * Being a sorted data structure, multisets offer efficient search operations.
 * A call to `txmultiset_find_tx()` looks-up an entry by a key. A multiset
 * can contain multiple entries with the same key. In this case
 * `txmultiset_find_tx()` returns one of them. The exact entry can vary
 * among calls.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          unsigned long key = 0;
 *
 *          struct txmultiset_entry* entry = txmultiset_size_tx(multiset, &key);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * The beginning and end of a range of entries with the same key is be
 * obtained by `txmultiset_lower_bound_tx()` and
 * `txmultiset_upper_bound_tx()`. The former returns the first entry with the
 * specified key; the latter returns the entry after the final entry with the
 * specified key. By iterating over the range, all entries with the key can
 * be obtained.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          unsigned int key_count = 0;
 *
 *          unsigned long key = 0;
 *
 *          struct txmultiset_entry* beg = txmultiset_lower_bound_tx(multiset, &key);
 *          struct txmultiset_entry* end = txmultiset_upper_bound_tx(multiset, &key);
 *
 *          while (beg != end) {
 *
 *              ++key_count;
 *
 *              beg = txmultiset_entry_next_tx(beg);
 *          }
 *
 *          // ... more transactional code ...
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * The above example counts the number of entries for the given key. The
 * function `txmultiset_count_tx()` performs this operation more efficiently.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          unsigned long key = 0;
 *
 *          unsigned int key_count = txmultiset_count_tx(multiset, &key);
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * Finally, to clear the whole multiset at once, there's
 * `txmultiset_clear_tx()`. It's equivalent to a continuous erase operation,
 * but prefered for its reduced overhead.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);
 *
 *          txmultiset_clear_tx(multiset);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 */
