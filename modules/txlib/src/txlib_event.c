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

#include "txlib_event.h"
#include <assert.h>
#include "picotm/picotm-error.h"
#include "txlist_tx.h"
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
        [TXLIB_QUEUE_PUSH] = undo_queue_push,
        [TXLIB_QUEUE_POP] = undo_queue_pop,
        [TXLIB_STACK_PUSH] = undo_stack_push,
        [TXLIB_STACK_POP] = undo_stack_pop
    };

    process_event(self, undo, error);
}
