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

#include <picotm/compiler.h>
#include <picotm/config/picotm-txlib-config.h>
#include <picotm/picotm-lib-rwlock.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txlist
 * \file
 * \brief Provides non-transactional state and entries for transactional lists
 */

/**
 * \brief Represents an entry in a transaction-safe list.
 */
struct txlist_entry {
    struct {
        struct txlist_entry* next;
        struct txlist_entry* prev;
    } internal;
};

/**
 * \brief Initializer macro for `struct txlist_entry`.
 *
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __TXLIST_ENTRY_INITIALIZER(_value)  \
    {                                       \
        {                                   \
            (_value),                       \
            (_value)                        \
        }                                   \
    }

/**
 * \brief Initializer macro for `struct txlist_entry`.
 */
#define TXLIST_ENTRY_INITIALIZER    __TXLIST_ENTRY_INITIALIZER(NULL)

PICOTM_NOTHROW
/**
 * \brief Initializes an entry of a transactional list.
 * \param self The list entry to initialize.
 */
void
txlist_entry_init(struct txlist_entry* self);

PICOTM_NOTHROW
/**
 * \brief Cleans up an entry of a transactional list.
 * \param self The list entry to clean up.
 */
void
txlist_entry_uninit(struct txlist_entry* self);

PICOTM_NOTHROW
/**
 * \brief Returns the next list entry.
 * \param self The current list entry.
 * \returns The next list entry.
 */
struct txlist_entry*
txlist_entry_next_tx(const struct txlist_entry* self);

PICOTM_NOTHROW
/**
 * \brief Returns the previous list entry.
 * \param self The current list entry.
 * \returns The previous list entry.
 */
struct txlist_entry*
txlist_entry_prev_tx(const struct txlist_entry* self);

/**
 * \brief The global state of transaction-safe list.
 */
struct txlist_state {
    struct {
        struct txlist_entry head;
        struct picotm_rwlock lock;
    } internal;
};

/**
 * \brief Initializer macro for `struct txlist_state`.
 */
#define TXLIST_STATE_INITIALIZER(list_state)                        \
    {                                                               \
        {                                                           \
            __TXLIST_ENTRY_INITIALIZER(&list_state.internal.head),  \
            PICOTM_RWLOCK_INITIALIZER                               \
        }                                                           \
    }

PICOTM_NOTHROW
/**
 * \brief Initializes list state.
 * \param self The list state to initialize.
 */
void
txlist_state_init(struct txlist_state* self);

PICOTM_NOTHROW
/**
 * \brief Cleans up list state.
 * \param self The list state to clean up.
 */
void
txlist_state_uninit(struct txlist_state* self);

PICOTM_NOTHROW
/**
 * \brief Removes all entries from a list state and runs a cleanup
 *        function on each.
 * \param self The list state to clear.
 * \param uninit The list-entry clean-up function.
 * \param data The clean-up function's data parameter.
 */
void
txlist_state_clear_and_uninit_entries(struct txlist_state* self,
                                      void (*uninit)(struct txlist_entry*,
                                                     void*),
                                      void* data);

PICOTM_END_DECLS
