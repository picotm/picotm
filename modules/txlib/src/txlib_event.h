/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

#include <stdbool.h>

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct txlist_entry;
struct txlist_tx;
struct txmultiset_entry;
struct txmultiset_tx;
struct txqueue_entry;
struct txqueue_tx;
struct txstack_entry;
struct txstack_tx;

/**
 * \brief Opcodes for operations on the data structures.
 */
enum txlib_op {
    /** \brief Represents an insert operation on a list. */
    TXLIB_LIST_INSERT,
    /** \brief Represents an erase operation on a list. */
    TXLIB_LIST_ERASE,
    /** \brief Represents an insert operation on a multiset. */
    TXLIB_MULTISET_INSERT,
    /** \brief Represents an erase operation on a multiset. */
    TXLIB_MULTISET_ERASE,
    /** \brief Represents an insert operation on a queue. */
    TXLIB_QUEUE_PUSH,
    /** \brief Represents an erase operation on a queue. */
    TXLIB_QUEUE_POP,
    /** \brief Represents an insert operation on a stack. */
    TXLIB_STACK_PUSH,
    /** \brief Represents an erase operation on a stack. */
    TXLIB_STACK_POP
};

/**
 * \brief Represents an event in a data-structure transaction.
 */
struct txlib_event {

    /** \brief The arguments of each operation. */
    union {
        struct {
            struct txlist_tx* list_tx;
            struct txlist_entry* entry;
        } list_insert;
        struct {
            struct txlist_tx* list_tx;
            struct txlist_entry* entry;
            struct txlist_entry* position;
        } list_erase;
        struct {
            struct txmultiset_tx* multiset_tx;
            struct txmultiset_entry* entry;
        } multiset_insert;
        struct {
            struct txmultiset_tx* multiset_tx;
            struct txmultiset_entry* entry;
        } multiset_erase;
        struct {
            struct txqueue_tx* queue_tx;
            struct txqueue_entry* entry;
        } queue_push;
        struct {
            struct txqueue_tx* queue_tx;
            struct txqueue_entry* entry;
            bool use_local_queue;
        } queue_pop;
        struct {
            struct txstack_tx* stack_tx;
            struct txstack_entry* entry;
        } stack_push;
        struct {
            struct txstack_tx* stack_tx;
            struct txstack_entry* entry;
            bool use_local_stack;
        } stack_pop;
    } arg;

    /** \brief The event's opcode. */
    enum txlib_op op;
};

void
txlib_event_apply(struct txlib_event* self, struct picotm_error* error);

void
txlib_event_undo(struct txlib_event* self, struct picotm_error* error);
