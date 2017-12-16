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
 */

#pragma once

#include <stddef.h>
#include <picotm/compiler.h>
#include <picotm/config/picotm-txlib-config.h>
#include <picotm/picotm-lib-rwlock.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txqueue
 * \file
 * \brief Provides non-transactional state and entries for transactional queues
 */

/**
 * \brief Represents an entry in a transaction-safe queue.
 */
struct txqueue_entry {
    struct {
        struct txqueue_entry* next;
        struct txqueue_entry* prev;
    } internal;
};

/**
 * \brief Initializer macro for `struct txqueue_entry`.
 *
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __TXQUEUE_ENTRY_INITIALIZER(_value) \
    {                                       \
        {                                   \
            (_value),                       \
            (_value)                        \
        }                                   \
    }

/**
 * \brief Initializer macro for `struct txqueue_entry`.
 */
#define TXQUEUE_ENTRY_INITIALIZER   __TXQUEUE_ENTRY_INITIALIZER(NULL)

PICOTM_NOTHROW
/**
 * \brief Initializes an entry of a transactional queue.
 * \param self The queue entry to initialize.
 */
void
txqueue_entry_init(struct txqueue_entry* self);

PICOTM_NOTHROW
/**
 * \brief Cleans up an entry of a transactional queue.
 * \param self The queue entry to clean up.
 */
void
txqueue_entry_uninit(struct txqueue_entry* self);

/**
 * \brief The global state of transaction-safe queue.
 */
struct txqueue_state {
    struct {
        struct txqueue_entry head;
        struct picotm_rwlock lock;
    } internal;
};

/**
 * \brief Initializer macro for `struct txqueue_state`.
 */
#define TXQUEUE_STATE_INITIALIZER(queue_state)                          \
    {                                                                   \
        {                                                               \
            __TXQUEUE_ENTRY_INITIALIZER(&(queue_state).internal.head),  \
            PICOTM_RWLOCK_INITIALIZER                                   \
        }                                                               \
    }

PICOTM_NOTHROW
/**
 * \brief Initializes queue state.
 * \param self The queue state to initialize.
 */
void
txqueue_state_init(struct txqueue_state* self);

PICOTM_NOTHROW
/**
 * \brief Cleans up queue state.
 * \param self The queue state to clean up.
 */
void
txqueue_state_uninit(struct txqueue_state* self);

PICOTM_NOTHROW
/**
 * \brief Removes all entries from a queue state and runs a cleanup
 *        function on each.
 * \param self The queue state to clear.
 * \param uninit The queue-entry clean-up function.
 * \param data The clean-up function's data parameter.
 */
void
txqueue_state_clear_and_uninit_entries(struct txqueue_state* self,
                                       void (*uninit)(struct txqueue_entry*,
                                                      void*),
                                       void* data);

PICOTM_END_DECLS
