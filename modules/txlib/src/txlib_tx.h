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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-slist.h"
#include <stddef.h>
#include <stdint.h>
#include "txlist_tx.h"
#include "txmultiset_tx.h"
#include "txqueue_tx.h"
#include "txstack_tx.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct txlib_event;

struct txlib_tx_entry {

    struct picotm_slist slist_entry;

    union {
        struct txlist_tx list_tx;
        struct txmultiset_tx multiset_tx;
        struct txqueue_tx queue_tx;
        struct txstack_tx stack_tx;
    } data;
};

static inline struct txlib_tx_entry*
txlib_tx_entry_of_slist(struct picotm_slist* item)
{
    return picotm_containerof(item, struct txlib_tx_entry, slist_entry);
}

static inline const struct txlib_tx_entry*
txlib_tx_entry_of_slist_const(const struct picotm_slist* item)
{
    return picotm_containerof(item, const struct txlib_tx_entry, slist_entry);
}

struct txlib_tx {
    unsigned long module;

    struct picotm_slist allocated_entries;
    struct picotm_slist acquired_list_tx;
    struct picotm_slist acquired_multiset_tx;
    struct picotm_slist acquired_queue_tx;
    struct picotm_slist acquired_stack_tx;

    struct txlib_event* event;
    size_t              nevents;
};

/**
 * \brief Initializes a data-structure transaction.
 * \param self The data-stucture transaction to initialize.
 * \param module The module number.
 */
void
txlib_tx_init(struct txlib_tx* self, unsigned long module);

/**
 * \brief Cleans up a data-structure transaction.
 */
void
txlib_tx_uninit(struct txlib_tx* self);

/**
 * \brief Acquires a list for use within a transaction.
 * \param self The data-structure transaction.
 * \param list_state The list state to acquire.
 * \param[out] error Returns an error to the caller.
 * \returns A pointer the new txlist transaction.
 */
struct txlist_tx*
txlib_tx_acquire_txlist_of_state(struct txlib_tx* self,
                                 struct txlist_state* list_state,
                                 struct picotm_error* error);

/**
 * \brief Acquires a multiset for use within a transaction.
 * \param self The data-structure transaction.
 * \param multiset_state The multiset state to acquire.
 * \param[out] error Returns an error to the caller.
 * \returns A pointer to the new txmultiset transaction.
 */
struct txmultiset_tx*
txlib_tx_acquire_txmultiset_of_state(struct txlib_tx* self,
                                     struct txmultiset_state* multiset_state,
                                     struct picotm_error* error);

/**
 * \brief Acquires a queue for use within a transaction.
 * \param self The data-structure transaction.
 * \param queue_state The queue state to acquire.
 * \param[out] error Returns an error to the caller.
 * \returns A pointer the new txqueue transaction.
 */
struct txqueue_tx*
txlib_tx_acquire_txqueue_of_state(struct txlib_tx* self,
                                  struct txqueue_state* queue_state,
                                  struct picotm_error* error);

/**
 * \brief Acquires a stack for use within a transaction.
 * \param self The data-structure transaction.
 * \param stack_state The stack state to acquire.
 * \param[out] error Returns an error to the caller.
 * \returns A pointer the new txstack transaction.
 */
struct txstack_tx*
txlib_tx_acquire_txstack_of_state(struct txlib_tx* self,
                                  struct txstack_state* stack_state,
                                  struct picotm_error* error);

/**
 * \brief Appends new events to the data-structure transaction's event log.
 * \param self The data-structure transaction.
 * \param nevents The number of events to append to the log.
 * \param init The event initializer function.
 * \param data1 The first data parameter.
 * \param[out] error Returns an error to the caller.
 */
void
txlib_tx_append_events1(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*,
                                     struct picotm_error*),
                        void* data1, struct picotm_error* error);

/**
 * \brief Appends new events to the data-structure transaction's event log.
 * \param self The data-structure transaction.
 * \param nevents The number of events to append to the log.
 * \param init The event initializer function.
 * \param data1 The first data parameter.
 * \param data2 The second data parameter.
 * \param[out] error Returns an error to the caller.
 */
void
txlib_tx_append_events2(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*, void*,
                                     struct picotm_error*),
                        void* data1, void* data2,
                        struct picotm_error* error);

/**
 * \brief Appends new events to the data-structure transaction's event log.
 * \param self The data-structure transaction.
 * \param nevents The number of events to append to the log.
 * \param init The event initializer function.
 * \param data1 The first data parameter.
 * \param data2 The second data parameter.
 * \param data3 The third data parameter.
 * \param[out] error Returns an error to the caller.
 */
void
txlib_tx_append_events3(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*, void*, void*,
                                     struct picotm_error*),
                        void* data1, void* data2, void* data3,
                        struct picotm_error* error);

/*
 * Module interface
 */

void
txlib_tx_prepare_commit(struct txlib_tx* self, struct picotm_error* error);

void
txlib_tx_apply_event(struct txlib_tx* self,
                     unsigned short op, uintptr_t cookie,
                     struct picotm_error* error);

void
txlib_tx_undo_event(struct txlib_tx* self,
                    unsigned short op, uintptr_t cookie,
                    struct picotm_error* error);

void
txlib_tx_finish(struct txlib_tx* self);
