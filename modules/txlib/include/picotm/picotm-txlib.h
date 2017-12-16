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

#include <picotm/config/picotm-txlib-config.h>
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
