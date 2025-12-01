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
#include "picotm-txlist.h"
#include "picotm-txlist-state.h"
#include "picotm-txqueue.h"
#include "picotm-txqueue-state.h"
#include "picotm-txmultiset.h"
#include "picotm-txmultiset-state.h"
#include "picotm-txstack.h"
#include "picotm-txstack-state.h"

/**
 * \ingroup group_txlib
 * \file
 * \brief Provides transactional data structures
 */

/**
 * \defgroup group_txlib The Transactional Data-Structures Module
 *
 * \brief The txlib module provides data structures that are safe to
 *        use from within transactions and cooperate with the transaction
 *        manager. Currently supported are lists, queues, multisets and
 *        stacks.
 *
 * Txlib, the transactional data-structures module, provides data structures
 * that are safe and efficient to use from within transactions. Each data
 * strcuture's implementation cooperates with the transaction manager to
 * provide concurrency control and error recovery.
 *
 * As a quick example, the following code removes an entry from a source
 * list and appends it to a destination list. The change becomes globally
 * visible during the transaction's successful commit. Concurrent,
 * conflicting access to the list data strcutures is detected and resolved
 * automatically. If the transaction has to roll back at any point, the
 * invoked operation are reverted and the lists return to their previous
 * state.
 *
 * ~~~ c
 *      struct txlist_state src_list_state = TXLIST_STATE_INITIALIZER(src_list_state);
 *      struct txlist_state dst_list_state = TXLIST_STATE_INITIALIZER(dst_list_state);
 *
 *      struct txlist_entry list_entry = TXLIST_ENTRY_INITIALIZER;
 *
 *      // init code
 *
 *      picotm_begin
 *
 *          struct txlist* src_list = txlist_of_state_tx(&src_list_state);
 *          struct txlist* dst_list = txlist_of_state_tx(&dst_list_state);
 *
 *          // The list state for 'src_list' already contains the entry.
 *          txlist_erase_tx(src_list, &list_entry);
 *
 *          txlist_push_back_tx(dst_list, &item->list_entry);
 *
 *          // ... more transactional code ...
 *
 *      picotm_commit
 *      picotm_end
 * ~~~
 *
 * For more information, refer to the documentation of the specific data
 * structure.
 *
 *  -# \ref group_txlib_txlist \n
 *      \copybrief group_txlib_txlist
 *  -# \ref group_txlib_txqueue \n
 *      \copybrief group_txlib_txqueue
 *  -# \ref group_txlib_txmultiset \n
 *      \copybrief group_txlib_txmultiset
 *  -# \ref group_txlib_txstack \n
 *      \copybrief group_txlib_txstack
 *
 */
