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

#include "picotm/config/picotm-txlib-config.h"
#include "picotm/compiler.h"
#include "picotm/picotm-lib-rwlock.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txlist
 * \file
 * \brief Provides non-transactional state and entries for transactional lists
 */

/**
 * \ingroup group_txlib
 * \brief Represents an entry in a transaction-safe list.
 */
struct txlist_entry {
    struct {
        struct txlist_entry* next;
        struct txlist_entry* prev;
    } internal;
};

/**
 * \ingroup group_txlib
 * \internal
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
 * \ingroup group_txlib
 * \brief Initializer macro for `struct txlist_entry`.
 */
#define TXLIST_ENTRY_INITIALIZER    __TXLIST_ENTRY_INITIALIZER(NULL)

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes an entry of a transactional list.
 * \param self The list entry to initialize.
 */
void
txlist_entry_init(struct txlist_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up an entry of a transactional list.
 * \param self The list entry to clean up.
 */
void
txlist_entry_uninit(struct txlist_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the next list entry.
 * \param self The current list entry.
 * \returns The next list entry.
 */
struct txlist_entry*
txlist_entry_next_tx(const struct txlist_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Returns the previous list entry.
 * \param self The current list entry.
 * \returns The previous list entry.
 */
struct txlist_entry*
txlist_entry_prev_tx(const struct txlist_entry* self);

/**
 * \ingroup group_txlib
 * \brief The global state of transaction-safe list.
 */
struct txlist_state {
    struct {
        struct txlist_entry head;
        struct picotm_rwlock lock;
    } internal;
};

/**
 * \ingroup group_txlib
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
 * \ingroup group_txlib
 * \brief Initializes list state.
 * \param self The list state to initialize.
 */
void
txlist_state_init(struct txlist_state* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up list state.
 * \param self The list state to clean up.
 */
void
txlist_state_uninit(struct txlist_state* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
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
