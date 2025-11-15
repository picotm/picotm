/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "picotm/config/picotm-txlib-config.h"
#include "picotm/compiler.h"
#include <stdbool.h>
#include <stddef.h>
#include "picotm-txstack-state.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txstack
 * \file
 * \brief Provides transactional stacks
 */

/**
 * \defgroup group_txlib_txstack Transactional Stacks
 *
 * \brief The transactional stack provides a transaction-safe
 *        implementation of a single-ended LIFO stack. Efficient insert
 *        and remove operations are supported on the same end.
 *
 * Stack entries are represented by `struct txstack_entry`. We can add an
 * instance of this data structure to any data value to turn it into a stack
 * entry. Here's an example for stacks of values of type `unsigned long`.
 *
 * ~~~ c
 *      struct ulong_item {
 *          struct txstack_entry stack_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      void
 *      ulong_item_init(struct ulong_item* item, unsigned long value)
 *      {
 *          txstack_entry_init(&item->stack_entry);
 *          item->value = value;
 *      }
 *
 *      struct ulong_item item;
 *
 *      ulong_item_init(&item, 0);
 * ~~~
 *
 * This code initializes the stack entry using `txstack_entry_init()`. The
 * macro `TXSTACK_ENTRY_INITIALIZER` initializes static or stack-allocated
 * stack entries. The example below illustrates this.
 *
 * ~~~
 *      struct ulong_item {
 *          struct txstack_entry stack_entry;
 *
 *          unsigned long value;
 *      };
 *
 *      #define ULONG_ITEM_INITIALIZER(_value)  \
 *      {                                       \
 *          TXSTACK_ENTRY_INITIALIZER,          \
 *          (_value)                            \
 *      }
 *
 *      struct ulong_item item = ULONG_ITEM_INITIALIZER(0);
 * ~~~
 *
 * When both, macro and function initializers, are possible, the macro
 * form is prefered. Stack entries are uninitialized with
 * `txstack_entry_uninit()`.
 *
 * ~~~ c
 *      void
 *      ulong_item_uninit(struct ulong_item* item)
 *      {
 *          txstack_entry_uninit(&item->stack_entry);
 *      }
 *
 *      ulong_item_uninit(&item);
 * ~~~
 *
 * To store the stack entries, we need non-transactional stack state, which
 * represents the shared state of a stack. It's implemented by
 * `struct txstack_state`. We can define and initialize a stack state
 * as illustrated in the example below.
 *
 * ~~~ c
 *      struct txstack_state stack_state;
 *
 *      txstack_state_init(&stack_state);
 * ~~~
 *
 * This code uses the initializer function `txstack_state_init()`. For
 * static or stack-allocated stack states, there's the initializer macro
 * `TXSTACK_STATE_INITIALIZER()`.
 *
 * ~~~ c
 *      struct txstack_state stack_state =
 *          TXSTACK_STATE_INITIALIZER(stack_state);
 * ~~~
 *
 * When both forms are possible, the initializer macro is prefered.
 *
 * Stack-state clean-up is performed by `txstack_state_uninit()`. The stack
 * state may not contain entries when the clean-up happens. This means that
 * entries have to be cleaned up from within a transaction.
 *
 * For many uses, this requirement just imposes unnecessary overhead. A call
 * to `txstack_state_clear_and_uninit_entries()` provies a non-transactional
 * way of clearing the stack from its entries. It erases each element from
 * the stack and calls a clean-up function on it. The example below shows
 * the clean-up code for a stack of `struct ulong_item` entries.
 *
 * ~~~ c
 *      struct ulong_item*
 *      ulong_item_of_entry(struct txstack_entry* entry)
 *      {
 *          return picotm_containerof(entry, struct ulong_item, stack_entry);
 *      }
 *
 *      void
 *      ulong_item_uninit_cb(struct txstack_entry* entry, void* data)
 *      {
 *          ulong_item_uninit(ulong_item_of_entry(entry));
 *
 *          // call free() if item was malloc()'ed
 *      }
 *
 *      txstack_state_clear_and_uninit_entries(&stack_state, ulong_item_uninit_cb, NULL);
 *      txstack_state_uninit(&stack_state);
 * ~~~
 *
 * At this point we have a stack state and stack entries to add to the state.
 * So far all code was non-transactional. Actual stack access and manipulation
 * is performed by transactional code.
 *
 * To perform stack operations within a transaction, we first need a stack
 * data structure for our transaction. It's represented by `struct txstack`.
 * A call to `txstack_of_state_tx()` returns an instance.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txstack* stack = txstack_of_state_tx(&stack_state);
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * Calling `txstack_of_state_tx()` multiple times for the same stack state
 * *within the same transaction* returns the same stack. Stacks are undefined
 * after their transaction committed or aborted, or within other, concurrent
 * transactions.
 *
 * With the stack, we can now put entries onto the stack state using
 * `txstack_push_tx()`. The example below illustrates this.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txstack* stack = txstack_of_state_tx(&stack_state);
 *
 *          txstack_push_tx(stack, &item->stack_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * After this transaction committed, the ulong data item `item` will be
 * the final entry in `stack_state`. If the transactions has to abort after
 * the call to `txstack_push_tx()`, the transaction framework will
 * automatically remove the pushed entry during the rollback; thus
 * restoring the original state.
 *
 * To remove the top-most entry from the stack, we can call `txstack_pop_tx()`.
 * This call is often combined with `txstack_top_tx()`, which returns the
 * stack's top-most entry. The combination is illustated in the example
 * below.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txstack* stack = txstack_of_state_tx(&stack_state);
 *
 *          // The stack state already contains the entry.
 *          struct txstack_entry* entry = txstack_top_tx(stack);
 *          txstack_pop_tx(stack);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * As usual, all errors are detected and handled by the transaction
 * framework. The benefits of transactional code show when we move
 * entries between stacks.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txstack* src_stack = txstack_of_state_tx(&src_stack_state);
 *          struct txstack* dst_stack = txstack_of_state_tx(&dst_stack_state);
 *
 *          // The stack state already contains the entry.
 *          struct txstack_entry* entry = txstack_top_tx(src_stack);
 *          txstack_pop_tx(src_stack);
 *
 *          txstack_push_tx(dst_stack, &item->stack_entry);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * In this example, we take an entry from a source stack and put it onto
 * a destination stack. The transaction framework automatically isolates these
 * operations from concurrent transactions until the transaction commits. So
 * concurrent transactions see the entry on *either* the source stack *or* the
 * destination stack, but never on both. If the transaction has to roll back
 * after the push operation, the transaction framework automatically removes
 * the stack entry from the destination stack and returns it to its old
 * position on the source stack.
 *
 * A call to `txstack_size_tx()` returns the number of stack entries, a call
 * to `txstack_empty_tx()` returns `true` if a stack is empty. The former
 * function might have linear complexity, the later function always has
 * constant complexity. It's therefore better to use `txstack_empty_tx()` if
 * it's only relevant whether there are entries.
 *
 * ~~~ c
 *      // init and ulong code here
 *
 *      picotm_begin
 *
 *          struct txstack* stack = txstack_of_state_tx(&stack_state);
 *
 *          bool is_empty = txstack_empty_tx(stack);
 *
 *          size_t size = txstack_size_tx(stack);
 *
 *          // more transactional code
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes an entry of a transactional stack from within a
 *        transaction.
 * \param self The stack entry to initialize.
 * \attention This function expects the entry's memory to be owned by
 *            the calling transaction. Shared-memory locations have to
 *            be read/write privatized first.
 */
void
txstack_entry_init_tm(struct txstack_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up an entry of a transactional stack from within a
 *        transaction.
 * \param self The stack entry to clean up.
 * \attention This function expects the entry's memory to be owned by
 *            the calling transaction. Shared-memory locations have to
 *            be read/write privatized first.
 */
void
txstack_entry_uninit_tm(struct txstack_entry* self);

/**
 * \ingroup group_txlib
 * \struct txstack
 * \brief A handle for operating on transaction-safe stacks.
 */
struct txstack;

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Creates a transactional stack for a stack state.
 * \param stack_state The stack state.
 * \returns A transactional stack for the stack state.
 */
struct txstack*
txstack_of_state_tx(struct txstack_state* stack_state);

/*
 * Capacity
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Tests a transactional stack for emptiness.
 * \param self The transactional stack.
 * \returns True if the stack is empty, false otherwise.
 */
bool
txstack_empty_tx(struct txstack* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the number of entries on a transactional stack.
 * \param self The transactional stack.
 * \returns The number of entries on the transactional stack.
 */
size_t
txstack_size_tx(struct txstack* self);

/*
 * Access
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the top-most entry of a transactional stack.
 * \param self The transactional stack.
 * \returns The entry at the transactional stack's front end.
 */
struct txstack_entry*
txstack_top_tx(struct txstack* self);

/*
 * Modfiers
 */

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes the top-most entry of a transactional stack.
 * \param self The transactional stack.
 */
void
txstack_pop_tx(struct txstack* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Inserts an entry at the top of a transactional stack.
 * \param self The transactional stack.
 * \param entry The stack entry to insert.
 */
void
txstack_push_tx(struct txstack* self, struct txstack_entry* entry);

PICOTM_END_DECLS
