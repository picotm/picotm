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
#include <stddef.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txqueue
 * \file
 * \brief Provides non-transactional state and entries for transactional queues
 */

/**
 * \ingroup group_txlib
 * \brief Represents an entry in a transaction-safe queue.
 */
struct txqueue_entry {
    struct {
        struct txqueue_entry* next;
        struct txqueue_entry* prev;
    } internal;
};

/**
 * \ingroup group_txlib
 * \internal
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
 * \ingroup group_txlib
 * \brief Initializer macro for `struct txqueue_entry`.
 */
#define TXQUEUE_ENTRY_INITIALIZER   __TXQUEUE_ENTRY_INITIALIZER(nullptr)

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes an entry of a transactional queue.
 * \param self The queue entry to initialize.
 */
void
txqueue_entry_init(struct txqueue_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up an entry of a transactional queue.
 * \param self The queue entry to clean up.
 */
void
txqueue_entry_uninit(struct txqueue_entry* self);

/**
 * \ingroup group_txlib
 * \brief The global state of transaction-safe queue.
 */
struct txqueue_state {
    struct {
        struct txqueue_entry head;
        struct picotm_rwlock lock;
    } internal;
};

/**
 * \ingroup group_txlib
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
 * \ingroup group_txlib
 * \brief Initializes queue state.
 * \param self The queue state to initialize.
 */
void
txqueue_state_init(struct txqueue_state* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up queue state.
 * \param self The queue state to clean up.
 */
void
txqueue_state_uninit(struct txqueue_state* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
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
