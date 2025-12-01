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

#include "picotm/compiler.h"
#include "picotm/config/picotm-txlib-config.h"
#include <stdbool.h>
#include <stddef.h>
#include "picotm-txlist-state.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txlist
 * \file
 * \brief Provides transactional lists
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes an entry of a transactional list from within a
 *        transaction.
 * \param self The list entry to initialize.
 * \attention This function expects the entry's memory to be owned by
 *            the calling transaction. Shared-memory locations have to
 *            be read/write privatized first.
 */
void
txlist_entry_init_tm(struct txlist_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up an entry of a transactional list from within a
 *        transaction.
 * \param self The list entry to clean up.
 * \attention This function expects the entry's memory to be owned by
 *            the calling transaction. Shared-memory locations have to
 *            be read/write privatized first.
 */
void
txlist_entry_uninit_tm(struct txlist_entry* self);

/**
 * \ingroup group_txlib
 * \struct txlist
 * \brief A handle for operating on transaction-safe lists.
 */
struct txlist;

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Creates a transactional list for a list state.
 * \param list_state The list state.
 * \returns A transactional list for the list state.
 */
struct txlist*
txlist_of_state_tx(struct txlist_state* list_state);

/*
 * Entries
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the first entry of a transactional list.
 * \param self The transactional list.
 * \returns The transactional list's first entry.
 */
struct txlist_entry*
txlist_begin_tx(struct txlist* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the terminator of a transactional list.
 * \param self The transactional list.
 * \returns The transactional list's terminator entry.
 */
struct txlist_entry*
txlist_end_tx(struct txlist* self);

/*
 * Capacity
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Tests a transactional list for emptiness.
 * \param self The transactional list.
 * \returns True if the list is empty, false otherwise.
 */
bool
txlist_empty_tx(struct txlist* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the number of entries in a transactional list.
 * \param self The transactional list.
 * \returns The number of entries in the transactional list.
 */
size_t
txlist_size_tx(struct txlist* self);

/*
 * Element access
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the front-end entry of a transactional list without
 *        removing it.
 * \param self The transactional list.
 * \returns The front entry.
 */
struct txlist_entry*
txlist_front_tx(struct txlist* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the back-end entry of a transactional list without
 *        removing it.
 * \param self The transactional list.
 * \returns The front entry.
 */
struct txlist_entry*
txlist_back_tx(struct txlist* self);

/*
 * Modfiers
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes all entries from a transactional list.
 * \param self The transactional list.
 */
void
txlist_clear_tx(struct txlist* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes an entry from a transactional list.
 * \param self The transactional list.
 * \param entry The list entry to remove.
 */
void
txlist_erase_tx(struct txlist* self, struct txlist_entry* entry);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Inserts an entry into a transactional list.
 * \param self The transactional list.
 * \param entry The list entry to insert.
 * \param position The list entry before which the new entry is inserted.
 */
void
txlist_insert_tx(struct txlist* self, struct txlist_entry* entry,
                 struct txlist_entry* position);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes the last entry of a transactional list.
 * \param self The transactional list.
 */
void
txlist_pop_back_tx(struct txlist* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes the first entry of a transactional list.
 * \param self The transactional list.
 */
void
txlist_pop_front_tx(struct txlist* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Inserts an entry at the end of a transactional list.
 * \param self The transactional list.
 * \param entry The list entry to insert.
 */
void
txlist_push_back_tx(struct txlist* self, struct txlist_entry* entry);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Inserts an entry at the beginning of a transactional list.
 * \param self The transactional list.
 * \param entry The list entry to insert.
 */
void
txlist_push_front_tx(struct txlist* self, struct txlist_entry* entry);

PICOTM_END_DECLS

/**
 * \defgroup group_txlib_txlist Transactional Lists
 *
 * \brief The transactional list provides a transaction-safe
 *        implementation of a double-linked list. Efficient insert
 *        and remove operations are supported anywhere within the
 *        list.
 *
 * The `struct txlist` data structure represents a transactional
 * double-linked list. To create a list, we first need list entries. These
 * are represented by `struct txlist_entry`. We can add an instance of this
 * data structure to any data value to turn it into a list entry. Here's an
 * example for lists of values of type `unsigned long`.
 *
 * ~~~ c
 *      struct ulong_item {
 *          struct txlist_entry list_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      void
 *      ulong_item_init(struct ulong_item* item, unsigned long value)
 *      {
 *          txlist_entry_init(&item->list_entry);
 *          item->value = value;
 *      }
 *
 *      struct ulong_item item;
 *
 *      ulong_item_init(&item, 0);
 * ~~~
 *
 * This code initializes the list entry using `txlist_entry_init()`. The
 * macro `TXLIST_ENTRY_INITIALIZER` initializes static or stack-allocated
 * list entries. The example below illustrates this.
 *
 * ~~~
 *      struct ulong_item {
 *          struct txlist_entry list_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      #define ULONG_ITEM_INITIALIZER(_value)  \
 *      {                                       \
 *          TXLIST_ENTRY_INITIALIZER,           \
 *          (_value)                            \
 *      }
 *
 *      struct ulong_item item = ULONG_ITEM_INITIALIZER(0);
 * ~~~
 *
 * When both, macro and function initialization, is possible, the macro
 * form is prefered. List entries are uninitialized with
 * `txlist_entry_uninit()`.
 *
 * ~~~ c
 *      void
 *      ulong_item_uninit(struct ulong_item* item)
 *      {
 *          txlist_entry_uninit(&item->list_entry);
 *      }
 *
 *      ulong_item_uninit(&item);
 * ~~~
 *
 * To store the list entries, we need non-transactional list state, which
 * represents the shared state of a double-linked list. It's implemented
 * by `struct txlist_state`. We can define and initialize a list state
 * as illustrated in the example below.
 *
 * ~~~ c
 *      struct txlist_state list_state;
 *
 *      txlist_state_init(&list_state);
 * ~~~
 *
 * This code uses the initializer function `txlist_state_init()`. For
 * static or stack-allocated list states, there's the initializer macro
 * `TXLIST_STATE_INITIALIZER()`.
 *
 * ~~~ c
 *      struct txlist_state list_state = TXLIST_STATE_INITIALIZER(list_state);
 * ~~~
 *
 * When both forms are possible, the initializer macro is the prefered form.
 *
 * List-state clean up is performed by `txlist_state_uninit()`. The list
 * state may not contain entries when the clean-up happens. This means that
 * entries have to be cleaned up from within a transaction.
 *
 * For many uses this requirement just imposes unnecessary overhead. A call
 * to `txlist_state_clear_and_uninit_entries()` provies a non-transactional
 * way of clearing the list from its entries. It erases each element from
 * the list and calls a clean-up function on it. The example below shows
 * the clean-up code for a list of `struct ulong_item` entries.
 *
 * ~~~ c
 *      struct ulong_item*
 *      ulong_item_of_entry(struct txlist_entry* entry)
 *      {
 *          return picotm_containerof(entry, struct ulong_item, list_entry);
 *      }
 *
 *      void
 *      ulong_item_uninit_cb(struct txlist_entry* entry, void* data)
 *      {
 *          ulong_item_uninit(ulong_item_of_entry(entry));
 *
 *          // call free() if item was malloc()'ed
 *      }
 *
 *      txlist_state_clear_and_uninit_entries(&list_state, ulong_item_uninit_cb, nullptr);
 *      txlist_state_uninit(&list_state);
 * ~~~
 *
 * At this point we have a list state and list entries to add to the state.
 * So far all code was non-transactional. Actual list access and manipulation
 * is performed by transactional code.
 *
 * To perform list operations within a transaction, we first need a list
 * data structure for our transaction. It's represented by `struct txlist`.
 * A call to `txlist_of_state_tx()` returns an instance.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txlist* list = txlist_of_state_tx(&list_state);
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * Calling `txlist_of_state_tx()` multiple times for the same list state
 * *within the same transaction* returns the same list. Lists are undefined
 * after their transaction committed or aborted, or within other, concurrent
 * transactions.
 *
 * With the list, we can now append or prepend entries to the list state
 * using `txlist_push_back_tx()` and `txlist_push_front_tx()`. The example
 * below illustrates the use of `txlist_push_back_tx()`.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txlist* list = txlist_of_state_tx(&list_state);
 *
 *          txlist_push_back_tx(list, &item->list_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * After this transaction committed, the ulong data item `item` will be
 * the back-end entry in `list_state`. If the transactions has to abort after
 * the call to `txlist_push_back_tx()`, the transaction framework will
 * automatically remove the appended entry during the rollback; thus
 * restoring the original state.
 *
 * To remove an entry from the list, we can call `txlist_erase_tx()`, as
 * illustated in the example below.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txlist* list = txlist_of_state_tx(&list_state);
 *
 *          // The list state already contains the entry.
 *          txlist_erase_tx(list, &item->list_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * As usual, all errors are detected and handled by the transaction
 * framework. The benefits of transactional code show when we move
 * entries between lists.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txlist* src_list = txlist_of_state_tx(&src_list_state);
 *          struct txlist* dst_list = txlist_of_state_tx(&dst_list_state);
 *
 *          // The list state already contains the entry.
 *          txlist_erase_tx(src_list, &item->list_entry);
 *
 *          txlist_push_back_tx(dst_list, &item->list_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * In this example, we remove an entry from a source list and append it to
 * a destination list. The transaction framework automatically isolates these
 * operations from concurrent transactions until the transaction commits. So
 * concurrent transactions see the entry in *either* the source list *or* the
 * destination list, but never in both. If the transaction has to roll back
 * after the append operation, the transaction framework automatically removes
 * the list entry from the destination list and returns it to its old position
 * in the source list.
 *
 * A call to `txlist_size_tx()` returns the number of list entries, a call to
 * `txlist_empty_tx()` returns `true` if a list is empty. The former function
 * might have linear complexity, the later function always has constant
 * complexity. It's therefore better to use `txlist_empty_tx()` if it's only
 * relevant whether there are entries.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txlist* list = txlist_of_state_tx(&list_state);
 *
 *          bool is_empty = txlist_empty_tx(list);
 *
 *          size_t size = txlist_size_tx(list);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * If we known that their are entries in the list, calls to
 * `txlist_front_tx()` and `txlist_back_tx()` return the front-end, resp. the
 * back-end entry.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          if (!txlist_empty_tx(tx)) {
 *
 *              struct txlist_entry* entry = txlist_front_tx()
 *
 *              // more transactional code
 *          }
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * We can also iterate over the entries of a list. The first entry of the list
 * is returned by `txlist_begin_tx()`. The terminator entry *after* the final
 * entry is returned by `txlist_end_tx()`. The terminator entry is not a real
 * entry and should not be dereferenced. Calls to `txlist_entry_next_tx()` and
 * `txlist_entry_prev_tx()` return an entry's successor or predecessor. Here's
 * and example that iterates over a list of values of type `unsigned long` and
 * sums up the individual values.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      unsigned long g_sum; // global variable containg sum of list entries
 *
 *      picotm_begin
 *
 *          struct txlist* list = txlist_of_state_tx(&list_state);
 *
 *          unsigned long sum = 0;
 *
 *          struct txlist_entry* beg = txlist_begin_tx(list);
 *          struct txlist_entry* end = txlist_end_tx(list);
 *
 *          while (beg != end) {
 *
 *              struct ulong_item* item = ulong_item_of_entry(beg);
 *
 *              sum += item->value;
 *
 *              beg = txlist_entry_next_tx(beg);
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
 * If we have an entry in a list, we can insert another entry *before*
 * the existing one with `txlist_insert_tx()`. In the example below, we
 * insert before the terminator entry, which is equivalent to an append
 * operation.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txlist* list = txlist_of_state_tx(&list_state);
 *
 *          txlist_insert_tx(list, &item->list_entry, txlist_end_tx(list));
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * To append or prepend entries to a list, it's better use the appropriate
 * push functions, though. The transaction framework might be able to optimize
 * concurrency control for each. Also, inserting or removing from a list
 * can make iteration loops invalid. It's therefore better to separate these
 * operations cleanly.
 *
 * Finally, to clear the whole list at once, there's `txlist_clear_tx()`.
 * It's equivalent to a continuous erase operation, but prefered for its
 * reduced overhead.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txlist* list = txlist_of_state_tx(&list_state);
 *
 *          txlist_clear_tx(list);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 */
