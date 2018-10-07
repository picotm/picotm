/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
 * \ingroup group_txlib_txstack
 * \file
 * \brief Provides non-transactional state and entries for transactional stacks
 */

/**
 * \ingroup group_txlib
 * \brief Represents an entry in a transaction-safe stack.
 */
struct txstack_entry {
    struct {
        struct txstack_entry* prev;
    } internal;
};

/**
 * \ingroup group_txlib
 * \internal
 * \brief Initializer macro for `struct txstack_entry`.
 *
 * \warning This is an internal interface. Don't use it in application code.
 */
#define __TXSTACK_ENTRY_INITIALIZER(_value) \
    {                                       \
        {                                   \
            (_value)                        \
        }                                   \
    }

/**
 * \ingroup group_txlib
 * \brief Initializer macro for `struct txstack_entry`.
 */
#define TXSTACK_ENTRY_INITIALIZER   __TXSTACK_ENTRY_INITIALIZER(NULL)

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes an entry of a transactional stack.
 * \param self The stack entry to initialize.
 */
void
txstack_entry_init(struct txstack_entry* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up an entry of a transactional stack.
 * \param self The stack entry to clean up.
 */
void
txstack_entry_uninit(struct txstack_entry* self);

/**
 * \ingroup group_txlib
 * \brief The global state of transaction-safe stack.
 */
struct txstack_state {
    struct {
        struct txstack_entry head;
        struct picotm_rwlock lock;
    } internal;
};

/**
 * \ingroup group_txlib
 * \brief Initializer macro for `struct txstack_state`.
 */
#define TXSTACK_STATE_INITIALIZER(stack_state)                          \
    {                                                                   \
        {                                                               \
            __TXSTACK_ENTRY_INITIALIZER(&(stack_state).internal.head),  \
            PICOTM_RWLOCK_INITIALIZER                                   \
        }                                                               \
    }

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Initializes stack state.
 * \param self The stack state to initialize.
 */
void
txstack_state_init(struct txstack_state* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Cleans up stack state.
 * \param self The stack state to clean up.
 */
void
txstack_state_uninit(struct txstack_state* self);

PICOTM_NOTHROW
/**
 * \ingroup group_txlib
 * \brief Removes all entries from a stack state and runs a cleanup
 *        function on each.
 * \param self The stack state to clear.
 * \param uninit The stack-entry clean-up function.
 * \param data The clean-up function's data parameter.
 */
void
txstack_state_clear_and_uninit_entries(struct txstack_state* self,
                                       void (*uninit)(struct txstack_entry*,
                                                      void*),
                                       void* data);

PICOTM_END_DECLS
