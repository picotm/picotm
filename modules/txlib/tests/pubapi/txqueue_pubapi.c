/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2019  Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "picotm/picotm.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-txqueue.h"
#include <stdlib.h>
#include <string.h>
#include "ptr.h"
#include "safeblk.h"
#include "safe_sched.h"
#include "safe_stdio.h"
#include "safe_stdlib.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

#define QUEUE_MAXNITEMS  16
#define QUEUE_BASE_VALUE 0

static struct txqueue_state g_queue_state;

struct ulong_queue_item {
    struct txqueue_entry queue_entry;

    unsigned long value;
};

static struct ulong_queue_item*
ulong_queue_item_of_queue_entry(struct txqueue_entry* queue_entry)
{
    return picotm_containerof(queue_entry, struct ulong_queue_item,
                              queue_entry);
}

static void
ulong_queue_item_init_with_value(struct ulong_queue_item* self,
                                 unsigned long value)
{
    txqueue_entry_init(&self->queue_entry);
    self->value = value;
}

static void
ulong_queue_item_uninit(struct ulong_queue_item* self)
{
    txqueue_entry_uninit(&self->queue_entry);
}

static void
init_ulong_queue_items(struct ulong_queue_item* beg,
                 const struct ulong_queue_item* end, unsigned long base_value)
{
    for (struct ulong_queue_item* pos = beg; pos < end; ++pos) {
        ulong_queue_item_init_with_value(
            pos, base_value + (unsigned long)(pos - beg));
    }
}

static void
uninit_txqueue_entry_cb(struct txqueue_entry* entry, void* data)
{
    ulong_queue_item_uninit(ulong_queue_item_of_queue_entry(entry));
}

/*
 * Declare queue state and acquire queue.
 */

static void
txqueue_test_1(unsigned int tid)
{
    struct txqueue_state queue_state = TXQUEUE_STATE_INITIALIZER(queue_state);

    picotm_begin

        struct txqueue* queue = txqueue_of_state_tx(&queue_state);

        if (!queue) {
            tap_error("condition failed: no queue");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txqueue_state_uninit(&queue_state);
}

/*
 * Push items into local queue.
 */

static void
txqueue_test_2(unsigned int tid)
{
    struct txqueue_state queue_state = TXQUEUE_STATE_INITIALIZER(queue_state);

    struct ulong_queue_item ulong_item[QUEUE_MAXNITEMS];
    init_ulong_queue_items(ulong_item,
                           ulong_item + picotm_arraylen(ulong_item),
                           QUEUE_BASE_VALUE);

    picotm_begin

        struct txqueue* queue = txqueue_of_state_tx(&queue_state);

        /* Insert all ulong items to queue */
        {
            struct ulong_queue_item* beg = ulong_item;
            struct ulong_queue_item* end = ulong_item + picotm_arraylen(ulong_item);

            for (struct ulong_queue_item* pos = beg; pos < end; ++pos) {
                txqueue_push_tx(queue, &pos->queue_entry);
            }
        }

        /* Compare front-end item with expected value; should be equal */
        {
            unsigned long value = QUEUE_BASE_VALUE;

            struct txqueue_entry* entry = txqueue_front_tx(queue);

            const struct ulong_queue_item* item =
                ulong_queue_item_of_queue_entry(entry);

            if (item->value != value) {
                tap_error("condition failed: value == item->value");
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
            }
        }

        /* Compare back-end item with expected value; should be equal */
        {
            unsigned long value = QUEUE_BASE_VALUE +
                                  picotm_arraylen(ulong_item) - 1;

            struct txqueue_entry* entry = txqueue_back_tx(queue);

            const struct ulong_queue_item* item =
                ulong_queue_item_of_queue_entry(entry);

            if (item->value != value) {
                tap_error("condition failed: value == item->value");
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txqueue_state_clear_and_uninit_entries(&queue_state,
                                           uninit_txqueue_entry_cb,
                                           NULL);
    txqueue_state_uninit(&queue_state);
}

/*
 * Remove items from local queue.
 */

static void
txqueue_test_3(unsigned int tid)
{
    struct txqueue_state queue_state = TXQUEUE_STATE_INITIALIZER(queue_state);

    struct ulong_queue_item ulong_item[QUEUE_MAXNITEMS];
    init_ulong_queue_items(picotm_arraybeg(ulong_item),
                           picotm_arrayend(ulong_item),
                           QUEUE_BASE_VALUE);

    picotm_begin

        struct txqueue* queue = txqueue_of_state_tx(&queue_state);

        /* Insert all ulong items to queue */
        {
            struct ulong_queue_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_queue_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_queue_item* pos = beg; pos < end; ++pos) {
                txqueue_push_tx(queue, &pos->queue_entry);
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    picotm_begin

        struct txqueue* queue = txqueue_of_state_tx(&queue_state);

        if (txqueue_empty_tx(queue)) {
            tap_error("condition failed: queue is non-empty before removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        if (txqueue_size_tx(queue) != picotm_arraylen(ulong_item)) {
            tap_error("condition failed: queue contains all elements");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        /* Remove all items from queue. */
        {
            struct ulong_queue_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_queue_item* end = picotm_arrayend(ulong_item);

            unsigned long value = QUEUE_BASE_VALUE;

            for (struct ulong_queue_item* pos = beg; pos < end; ++pos, ++value) {

                struct txqueue_entry* entry = txqueue_front_tx(queue);
                txqueue_pop_tx(queue);

                const struct ulong_queue_item* item =
                    ulong_queue_item_of_queue_entry(entry);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }

            }
        }

        if (!txqueue_empty_tx(queue)) {
            tap_error("condition failed: queue is empty after removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txqueue_state_uninit(&queue_state);
}

/*
 * Insert items into shared queue
 */

static void
txqueue_test_4(unsigned int tid)
{
    struct ulong_queue_item* ulong_item[QUEUE_MAXNITEMS];

    {
        struct ulong_queue_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_queue_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_queue_item** pos = beg; pos != end; ++pos) {
            struct ulong_queue_item* item = safe_malloc(sizeof(*item));
            ulong_queue_item_init_with_value(item, tid);
            *pos = item;
        }
    }

    /* Insert all ulong items to queue */

    picotm_begin

        struct txqueue* queue = txqueue_of_state_tx(&g_queue_state);

        struct ulong_queue_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_queue_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_queue_item** pos = beg; pos != end; ++pos) {
            txqueue_push_tx(queue, &(*pos)->queue_entry);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    safe_sched_yield();

    /* Compare values; consecutive items should have same value. */

    picotm_begin

        struct txqueue* queue = txqueue_of_state_tx(&g_queue_state);

        picotm_safe unsigned long value;

        for (size_t i = 0; i < picotm_arraylen(ulong_item); ++i) {

            struct txqueue_entry* entry = txqueue_front_tx(queue);

            struct ulong_queue_item* item =
                ulong_queue_item_of_queue_entry(entry);

            if (!i) {
                value = item->value;
            } else if (item->value != value) {
                tap_error("condition failed: value == item->value");
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
            }

            /* TM not required here */
            ulong_item[i] = item;

            txqueue_pop_tx(queue);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    /* Free items */

    {
        struct ulong_queue_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_queue_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_queue_item** pos = beg; pos != end; ++pos) {
            ulong_queue_item_uninit(*pos);
            free(*pos);
        }
    }
}

static void
txqueue_test_4_pre(unsigned long nthreads, enum loop_mode loop,
                   enum boundary_type btype, unsigned long long bound)
{
    txqueue_state_init(&g_queue_state);
}

static void
txqueue_test_4_post(unsigned long nthreads, enum loop_mode loop,
                    enum boundary_type btype, unsigned long long bound)
{
    txqueue_state_uninit(&g_queue_state);
}

static const struct test_func txqueue_test[] = {
    {"txqueue_test_1", txqueue_test_1, NULL,               NULL},
    {"txqueue_test_2", txqueue_test_2, NULL,               NULL},
    {"txqueue_test_3", txqueue_test_3, NULL,               NULL},
    {"txqueue_test_4", txqueue_test_4, txqueue_test_4_pre, txqueue_test_4_post}
};

/*
 * Entry point
 */

#include "opts.h"
#include "pubapi.h"

int
main(int argc, char* argv[])
{
    return pubapi_main(argc, argv, PARSE_OPTS_STRING(),
                       txqueue_test, arraylen(txqueue_test));
}
