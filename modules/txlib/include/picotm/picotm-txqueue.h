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

#pragma once

#include <picotm/compiler.h>
#include <picotm/config/picotm-txlib-config.h>
#include <stdbool.h>
#include <stddef.h>
#include "picotm-txqueue-state.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txqueue
 * \file
 * \brief Provides transactional queues
 */

/**
 * \struct txqueue
 * \brief A handle for operating on transaction-safe queues.
 */
struct txqueue;

PICOTM_NOTHROW
/**
 * \brief Creates a transactional queue for a queue state.
 * \param queue_state The queue state.
 * \returns A transactional queue for the queue state.
 */
struct txqueue*
txqueue_of_state_tx(struct txqueue_state* queue_state);

/*
 * Capacity
 */

PICOTM_NOTHROW
/**
 * \brief Tests a transactional queue for emptiness.
 * \param self The transactional queue.
 * \returns True if the queue is empty, false otherwise.
 */
bool
txqueue_empty_tx(struct txqueue* self);

PICOTM_NOTHROW
/**
 * \brief Returns the number of entries in a transactional queue.
 * \param self The transactional queue.
 * \returns The number of entries in the transactional queue.
 */
size_t
txqueue_size_tx(struct txqueue* self);

/*
 * Access
 */

PICOTM_NOTHROW
/**
 * \brief Returns the front-end entry of a transactional queue.
 * \param self The transactional queue.
 * \returns The entry at the transactional queue's front end.
 */
struct txqueue_entry*
txqueue_front_tx(struct txqueue* self);

PICOTM_NOTHROW
/**
 * \brief Retruns the back-end entry of a transactional queue.
 * \param self The transactional queue.
 * \returns The entry at the transactional queue's back end.
 */
struct txqueue_entry*
txqueue_back_tx(struct txqueue* self);

/*
 * Modfiers
 */

PICOTM_NOTHROW
/**
 * \brief Removes the last entry of a transactional queue.
 * \param self The transactional queue.
 */
void
txqueue_pop_tx(struct txqueue* self);

PICOTM_NOTHROW
/**
 * \brief Inserts an entry at the front of a transactional queue.
 * \param self The transactional queue.
 * \param entry The queue entry to insert.
 */
void
txqueue_push_tx(struct txqueue* self, struct txqueue_entry* entry);

PICOTM_END_DECLS

/**
 * \defgroup group_txlib_txqueue Transactional Queues
 *
 * \brief The transactional queue provides a transaction-safe
 *        implementation of a single-ended FIFO queue. Efficient insert
 *        operations are supported on one end, efficient remove operations
 *        are supported on the opposite end.
 *
 * Queue entries are represented by `struct txqueue_entry`. We can add an
 * instance of this data structure to any data value to turn it into a queue
 * entry. Here's an example for queues of values of type `unsigned long`.
 *
 * ~~~ c
 *      struct ulong_item {
 *          struct txqueue_entry queue_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      void
 *      ulong_item_init(struct ulong_item* item, unsigned long value)
 *      {
 *          txqueue_entry_init(&item->queue_entry);
 *          item->value = value;
 *      }
 *
 *      struct ulong_item item;
 *
 *      ulong_item_init(&item, 0);
 * ~~~
 *
 * This code initializes the queue entry using `txqueue_entry_init()`. The
 * macro `TXQUEUE_ENTRY_INITIALIZER` initializes static or stack-allocated
 * queue entries. The example below illustrates this.
 *
 * ~~~
 *      struct ulong_item {
 *          struct txqueue_entry queue_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      #define ULONG_ITEM_INITIALIZER(_value)  \
 *      {                                       \
 *          TXQUEUE_ENTRY_INITIALIZER,          \
 *          (_value)                            \
 *      }
 *
 *      struct ulong_item item = ULONG_ITEM_INITIALIZER(0);
 * ~~~
 *
 * When both, macro and function initializers, are possible, the macro
 * form is prefered. Queue entries are uninitialized with
 * `txqueue_entry_uninit()`.
 *
 * ~~~ c
 *      void
 *      ulong_item_uninit(struct ulong_item* item)
 *      {
 *          txqueue_entry_uninit(&item->queue_entry);
 *      }
 *
 *      ulong_item_uninit(&item);
 * ~~~
 *
 * To store the queue entries, we need non-transactional queue state, which
 * represents the shared state of a queue. It's implemented by
 * `struct txqueue_state`. We can define and initialize a queue state
 * as illustrated in the example below.
 *
 * ~~~ c
 *      struct txqueue_state queue_state;
 *
 *      txqueue_state_init(&queue_state);
 * ~~~
 *
 * This code uses the initializer function `txqueue_state_init()`. For
 * static or stack-allocated queue states, there's the initializer macro
 * `TXQUEUE_STATE_INITIALIZER()`.
 *
 * ~~~ c
 *      struct txqueue_state queue_state =
 *          TXQUEUE_STATE_INITIALIZER(queue_state);
 * ~~~
 *
 * When both forms are possible, the initializer macro is prefered.
 *
 * Queue-state clean-up is performed by `txqueue_state_uninit()`. The queue
 * state may not contain entries when the clean-up happens. This means that
 * entries have to be cleaned up from within a transaction.
 *
 * For many uses, this requirement just imposes unnecessary overhead. A call
 * to `txqueue_state_clear_and_uninit_entries()` provies a non-transactional
 * way of clearing the queue from its entries. It erases each element from
 * the queue and calls a clean-up function on it. The example below shows
 * the clean-up code for a queue of `struct ulong_item` entries.
 *
 * ~~~ c
 *      struct ulong_item*
 *      ulong_item_of_entry(struct txqueue_entry* entry)
 *      {
 *          return picotm_containerof(entry, struct ulong_item, queue_entry);
 *      }
 *
 *      void
 *      ulong_item_uninit_cb(struct txqueue_entry* entry, void* data)
 *      {
 *          ulong_item_uninit(ulong_item_of_entry(entry));
 *
 *          // call free() if item was malloc()'ed
 *      }
 *
 *      txqueue_state_clear_and_uninit_entries(&queue_state, ulong_item_uninit_cb, NULL);
 *      txqueue_state_uninit(&queue_state);
 * ~~~
 *
 * At this point we have a queue state and queue entries to add to the state.
 * So far all code was non-transactional. Actual queue access and manipulation
 * is performed by transactional code.
 *
 * To perform queue operations within a transaction, we first need a queue
 * data structure for our transaction. It's represented by `struct txqueue`.
 * A call to `txqueue_of_state_tx()` returns an instance.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txqueue* queue = txqueue_of_state_tx(&queue_state);
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * Calling `txqueue_of_state_tx()` multiple times for the same queue state
 * *within the same transaction* returns the same queue. Queues are undefined
 * after their transaction committed or aborted, or within other, concurrent
 * transactions.
 *
 * With the queue, we can now append entries to the queue state using
 * `txqueue_push_tx()`. The example below illustrates this.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txqueue* queue = txqueue_of_state_tx(&queue_state);
 *
 *          txqueue_push_tx(queue, &item->queue_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * After this transaction committed, the ulong data item `item` will be
 * the final entry in `queue_state`. If the transactions has to abort after
 * the call to `txqueue_push_tx()`, the transaction framework will
 * automatically remove the appended entry during the rollback; thus
 * restoring the original state.
 *
 * To remove the next entry from the queue, we can call `txqueue_pop_tx()`.
 * This call is often combined with `txqueue_front_tx()`, which returns the
 * queue's front-end entry. The combination is illustated in the example
 * below.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txqueue* queue = txqueue_of_state_tx(&queue_state);
 *
 *          // The queue state already contains the entry.
 *          struct txqueue_entry* entry = txqueue_front_tx(queue);
 *          txqueue_pop_tx(queue);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * As usual, all errors are detected and handled by the transaction
 * framework. The benefits of transactional code show when we move
 * entries between queues.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txqueue* src_queue = txqueue_of_state_tx(&src_queue_state);
 *          struct txqueue* dst_queue = txqueue_of_state_tx(&dst_queue_state);
 *
 *          // The queue state already contains the entry.
 *          struct txqueue_entry* entry = txqueue_front_tx(src_queue);
 *          txqueue_pop_tx(src_queue);
 *
 *          txqueue_push_tx(dst_queue, &item->queue_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * In this example, we remove an entry from a source source and append it to
 * a destination queue. The transaction framework automatically isolates these
 * operations from concurrent transactions until the transaction commits. So
 * concurrent transactions see the entry in *either* the source queue *or* the
 * destination queue, but never in both. If the transaction has to roll back
 * after the append operation, the transaction framework automatically removes
 * the queue entry from the destination queue and returns it to its old
 * position in the source queue.
 *
 * A call to `txqueue_size_tx()` returns the number of queue entries, a call
 * to `txqueue_empty_tx()` returns `true` if a queue is empty. The former
 * function might have linear complexity, the later function always has
 * constant complexity. It's therefore better to use `txqueue_empty_tx()` if
 * it's only relevant whether there are entries.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txqueue* queue = txqueue_of_state_tx(&queue_state);
 *
 *          bool is_empty = txqueue_empty_tx(queue);
 *
 *          size_t size = txqueue_size_tx(queue);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 */
