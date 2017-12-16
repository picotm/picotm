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
#include <picotm/picotm-lib-rwlock.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txmultiset
 * \file
 * \brief Provides non-transactional state and entries for transactional
 *        multisets.
 */

/**
 * \brief Represents an entry in a transaction-safe multiset.
 */
struct txmultiset_entry {
    struct {
        struct txmultiset_entry* lt;
        struct txmultiset_entry* ge;
        struct txmultiset_entry* parent;
    } internal;
};

/**
 * \brief Initializer macro for `struct txmultiset_entry`.
 *
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __TXMULTISET_ENTRY_INITIALIZER(_parent) \
    {                                           \
        {                                       \
            NULL,                               \
            NULL,                               \
            (_parent)                           \
        }                                       \
    }

/**
 * \brief Initializer macro for `struct txmultiset_entry`.
 */
#define TXMULTISET_ENTRY_INITIALIZER    __TXMULTISET_ENTRY_INITIALIZER(NULL)

PICOTM_NOTHROW
/**
 * \brief Initializes an entry of a transactional multiset.
 * \param self The multiset entry to initialize.
 */
void
txmultiset_entry_init(struct txmultiset_entry* self);

PICOTM_NOTHROW
/**
 * \brief Cleans up an entry of a transactional multiset.
 * \param self The multiset entry to clean up.
 */
void
txmultiset_entry_uninit(struct txmultiset_entry* self);

PICOTM_NOTHROW
/**
 * \brief Returns the next multiset entry.
 * \param self The current multiset entry.
 * \returns The next multiset entry.
 */
struct txmultiset_entry*
txmultiset_entry_next_tx(const struct txmultiset_entry* self);

PICOTM_NOTHROW
/**
 * \brief Returns the previous multiset entry.
 * \param self The current multiset entry.
 * \returns The previous multiset entry.
 */
struct txmultiset_entry*
txmultiset_entry_prev_tx(const struct txmultiset_entry* self);

/**
 * \brief Generates a compare key for a multiset entry.
 * \param The multiset entry.
 * \returns A comparable key for the multiset entry.
 */
typedef const void* (*txmultiset_key_function)(struct txmultiset_entry* entry);

/**
 * \brief Key-compare function for two multiset-entry keys.
 * \param lhs The left-hand-side key.
 * \param rhs The right-hand-side key.
 * \returns A value less than, equal to, or greater than 0 if lhs is
 *          less than, equal to, or greater than rhs.
 */
typedef int (*txmultiset_compare_function)(const void* lhs, const void* rhs);

/**
 * \brief The global state of transaction-safe multiset.
 */
struct txmultiset_state {
    struct {
        struct txmultiset_entry head;
        struct picotm_rwlock lock;
        txmultiset_key_function key;
        txmultiset_compare_function compare;
    } internal;
};

/**
 * \brief Initializer macro for `struct txmultiset_state`.
 */
#define TXMULTISET_STATE_INITIALIZER(_multiset_state, _key, _compare)         \
    {                                                                         \
        {                                                                     \
            __TXMULTISET_ENTRY_INITIALIZER(&(_multiset_state).internal.head), \
            PICOTM_RWLOCK_INITIALIZER,                                        \
            (_key),                                                           \
            (_compare),                                                       \
        }                                                                     \
    }

PICOTM_NOTHROW
/**
 * \brief Initializes multiset state.
 * \param self The multiset state to initialize.
 * \param key The key generator function for the multiset's entries.
 * \param compare The key-compare function.
 */
void
txmultiset_state_init(struct txmultiset_state* self,
                      txmultiset_key_function key,
                      txmultiset_compare_function compare);

PICOTM_NOTHROW
/**
 * \brief Cleans up multiset state.
 * \param self The multiset state to clean up.
 */
void
txmultiset_state_uninit(struct txmultiset_state* self);

PICOTM_NOTHROW
/**
 * \brief Removes all entries from a multiset state and runs a cleanup
 *        function on each.
 * \param self The multiset state to clear.
 * \param uninit The multiset-entry clean-up function.
 * \param data The clean-up function's data parameter.
 */
void
txmultiset_state_clear_and_uninit_entries(struct txmultiset_state* self,
                                          void (*uninit)(
                                              struct txmultiset_entry*,
                                              void*),
                                          void* data);

PICOTM_END_DECLS
