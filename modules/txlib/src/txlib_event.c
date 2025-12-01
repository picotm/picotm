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

#include "txlib_event.h"
#include <assert.h>
#include "picotm/picotm-error.h"
#include "txlist_tx.h"
#include "txmultiset_tx.h"
#include "txqueue_tx.h"
#include "txstack_tx.h"

static void
process_event(struct txlib_event* event,
              void (* const op_func[])(struct txlib_event*,
                                       struct picotm_error*),
              struct picotm_error* error)
{
    assert(event);
    assert(op_func);

    if (!op_func[event->op]) {
        return;
    }

    op_func[event->op](event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Apply events
 */

static void
apply_queue_push(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_QUEUE_PUSH);

    txqueue_tx_apply_push(event->arg.queue_push.queue_tx,
                          event->arg.queue_push.entry,
                          error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
apply_stack_push(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_STACK_PUSH);

    txstack_tx_apply_push(event->arg.stack_push.stack_tx,
                          event->arg.stack_push.entry,
                          error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlib_event_apply(struct txlib_event* self, struct picotm_error* error)
{
    static void (* const apply[])(struct txlib_event*,
                                  struct picotm_error*) = {
        [TXLIB_LIST_INSERT] = NULL,
        [TXLIB_LIST_ERASE] = NULL,
        [TXLIB_MULTISET_INSERT] = NULL,
        [TXLIB_MULTISET_ERASE] = NULL,
        [TXLIB_QUEUE_PUSH] = apply_queue_push,
        [TXLIB_QUEUE_POP] = NULL,
        [TXLIB_STACK_PUSH] = apply_stack_push,
        [TXLIB_STACK_POP] = NULL
    };

    process_event(self, apply, error);
}

/*
 * Un-do events
 */

static void
undo_list_insert(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_LIST_INSERT);

    txlist_tx_undo_insert(event->arg.list_insert.list_tx,
                          event->arg.list_insert.entry,
                          error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_list_erase(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_LIST_ERASE);

    txlist_tx_undo_erase(event->arg.list_erase.list_tx,
                         event->arg.list_erase.entry,
                         event->arg.list_erase.position,
                         error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_multiset_insert(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_MULTISET_INSERT);

    txmultiset_tx_undo_insert(event->arg.multiset_insert.multiset_tx,
                              event->arg.multiset_insert.entry,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_multiset_erase(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_MULTISET_ERASE);

    txmultiset_tx_undo_erase(event->arg.multiset_erase.multiset_tx,
                             event->arg.multiset_erase.entry,
                             error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_queue_push(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_QUEUE_PUSH);

    txqueue_tx_undo_push(event->arg.queue_push.queue_tx,
                         event->arg.queue_push.entry,
                         error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_queue_pop(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_QUEUE_POP);

    txqueue_tx_undo_pop(event->arg.queue_pop.queue_tx,
                        event->arg.queue_pop.entry,
                        event->arg.queue_pop.use_local_queue,
                        error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_stack_push(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_STACK_PUSH);

    txstack_tx_undo_push(event->arg.stack_push.stack_tx,
                         event->arg.stack_push.entry,
                         error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
undo_stack_pop(struct txlib_event* event, struct picotm_error* error)
{
    assert(event);
    assert(event->op == TXLIB_STACK_POP);

    txstack_tx_undo_pop(event->arg.stack_pop.stack_tx,
                        event->arg.stack_pop.entry,
                        event->arg.stack_pop.use_local_stack,
                        error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlib_event_undo(struct txlib_event* self, struct picotm_error* error)
{
    static void (* const undo[])(struct txlib_event*,
                                 struct picotm_error*) = {
        [TXLIB_LIST_INSERT] = undo_list_insert,
        [TXLIB_LIST_ERASE] = undo_list_erase,
        [TXLIB_MULTISET_INSERT] = undo_multiset_insert,
        [TXLIB_MULTISET_ERASE] = undo_multiset_erase,
        [TXLIB_QUEUE_PUSH] = undo_queue_push,
        [TXLIB_QUEUE_POP] = undo_queue_pop,
        [TXLIB_STACK_PUSH] = undo_stack_push,
        [TXLIB_STACK_POP] = undo_stack_pop
    };

    process_event(self, undo, error);
}
