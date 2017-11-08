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
struct txqueue_entry;
struct txqueue_tx;

/**
 * \brief Opcodes for operations on the data structures.
 */
enum txlib_op {
    /** \brief Represents an insert operation on a list. */
    TXLIB_LIST_INSERT,
    /** \brief Represents an erase operation on a list. */
    TXLIB_LIST_ERASE,
    /** \brief Represents an insert operation on a queue. */
    TXLIB_QUEUE_PUSH,
    /** \brief Represents an erase operation on a queue. */
    TXLIB_QUEUE_POP
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
            struct txqueue_tx* queue_tx;
            struct txqueue_entry* entry;
        } queue_push;
        struct {
            struct txqueue_tx* queue_tx;
            struct txqueue_entry* entry;
            bool use_local_queue;
        } queue_pop;
    } arg;

    /** \brief The event's opcode. */
    enum txlib_op op;
};

void
txlib_event_apply(struct txlib_event* self, struct picotm_error* error);

void
txlib_event_undo(struct txlib_event* self, struct picotm_error* error);
