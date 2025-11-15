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
#include "picotm/picotm-lib-rwlock.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txmultiset
 * \file
 * \brief Provides non-transactional state and entries for transactional
 *        multisets.
 */

/**
 * \ingroup group_txlib
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
 * \ingroup group_txlib
 * \internal
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
 * \ingroup group_txlib
 * \brief Initializer macro for `struct txmultiset_entry`.
 */
#define TXMULTISET_ENTRY_INITIALIZER    __TXMULTISET_ENTRY_INITIALIZER(NULL)

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes an entry of a transactional multiset.
 * \param self The multiset entry to initialize.
 */
void
txmultiset_entry_init(struct txmultiset_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up an entry of a transactional multiset.
 * \param self The multiset entry to clean up.
 */
void
txmultiset_entry_uninit(struct txmultiset_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the next multiset entry.
 * \param self The current multiset entry.
 * \returns The next multiset entry.
 */
struct txmultiset_entry*
txmultiset_entry_next_tx(const struct txmultiset_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the previous multiset entry.
 * \param self The current multiset entry.
 * \returns The previous multiset entry.
 */
struct txmultiset_entry*
txmultiset_entry_prev_tx(const struct txmultiset_entry* self);

/**
 * \ingroup group_txlib
 * \brief Generates a compare key for a multiset entry.
 * \param The multiset entry.
 * \returns A comparable key for the multiset entry.
 */
typedef const void* (*txmultiset_key_function)(struct txmultiset_entry* entry);

/**
 * \ingroup group_txlib
 * \brief Key-compare function for two multiset-entry keys.
 * \param lhs The left-hand-side key.
 * \param rhs The right-hand-side key.
 * \returns A value less than, equal to, or greater than 0 if lhs is
 *          less than, equal to, or greater than rhs.
 */
typedef int (*txmultiset_compare_function)(const void* lhs, const void* rhs);

/**
 * \ingroup group_txlib
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
 * \ingroup group_txlib
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
 * \ingroup group_txlib
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
 * \ingroup group_txlib
 * \brief Cleans up multiset state.
 * \param self The multiset state to clean up.
 */
void
txmultiset_state_uninit(struct txmultiset_state* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
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
