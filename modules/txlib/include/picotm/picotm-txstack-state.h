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
#include <stddef.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_txlib
 * \ingroup group_txlib_txstack
 * \file
 * \brief Provides non-transactional state and entries for transactional stacks
 */

/**
 * \brief Represents an entry in a transaction-safe stack.
 */
struct txstack_entry {
    struct {
        struct txstack_entry* prev;
    } internal;
};

/**
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
 * \brief Initializer macro for `struct txstack_entry`.
 */
#define TXSTACK_ENTRY_INITIALIZER   __TXSTACK_ENTRY_INITIALIZER(NULL)

PICOTM_NOTHROW
/**
 * \brief Initializes an entry of a transactional stack.
 * \param self The stack entry to initialize.
 */
void
txstack_entry_init(struct txstack_entry* self);

PICOTM_NOTHROW
/**
 * \brief Cleans up an entry of a transactional stack.
 * \param self The stack entry to clean up.
 */
void
txstack_entry_uninit(struct txstack_entry* self);

/**
 * \brief The global state of transaction-safe stack.
 */
struct txstack_state {
    struct {
        struct txstack_entry head;
        struct picotm_rwlock lock;
    } internal;
};

/**
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
 * \brief Initializes stack state.
 * \param self The stack state to initialize.
 */
void
txstack_state_init(struct txstack_state* self);

PICOTM_NOTHROW
/**
 * \brief Cleans up stack state.
 * \param self The stack state to clean up.
 */
void
txstack_state_uninit(struct txstack_state* self);

PICOTM_NOTHROW
/**
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
