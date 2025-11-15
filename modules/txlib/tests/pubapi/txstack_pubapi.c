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
#include "picotm/picotm-txstack.h"
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

#define STACK_MAXNITEMS  16
#define STACK_BASE_VALUE 0

static struct txstack_state g_stack_state;

struct ulong_stack_item {
    struct txstack_entry stack_entry;

    unsigned long value;
};

static struct ulong_stack_item*
ulong_stack_item_of_stack_entry(struct txstack_entry* stack_entry)
{
    return picotm_containerof(stack_entry, struct ulong_stack_item,
                              stack_entry);
}

static void
ulong_stack_item_init_with_value(struct ulong_stack_item* self,
                                 unsigned long value)
{
    txstack_entry_init(&self->stack_entry);
    self->value = value;
}

static void
ulong_stack_item_uninit(struct ulong_stack_item* self)
{
    txstack_entry_uninit(&self->stack_entry);
}

static void
init_ulong_stack_items(struct ulong_stack_item* beg,
                 const struct ulong_stack_item* end, unsigned long base_value)
{
    for (struct ulong_stack_item* pos = beg; pos < end; ++pos) {
        ulong_stack_item_init_with_value(
            pos, base_value + (unsigned long)(pos - beg));
    }
}

static void
uninit_txstack_entry_cb(struct txstack_entry* entry, void* data)
{
    ulong_stack_item_uninit(ulong_stack_item_of_stack_entry(entry));
}

/*
 * Declare stack state and acquire stack.
 */

static void
txstack_test_1(unsigned int tid)
{
    struct txstack_state stack_state = TXSTACK_STATE_INITIALIZER(stack_state);

    picotm_begin

        struct txstack* stack = txstack_of_state_tx(&stack_state);

        if (!stack) {
            tap_error("condition failed: no stack");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txstack_state_uninit(&stack_state);
}

/*
 * Push items onto local stack.
 */

static void
txstack_test_2(unsigned int tid)
{
    struct txstack_state stack_state = TXSTACK_STATE_INITIALIZER(stack_state);

    struct ulong_stack_item ulong_item[STACK_MAXNITEMS];
    init_ulong_stack_items(ulong_item,
                           ulong_item + picotm_arraylen(ulong_item),
                           STACK_BASE_VALUE);

    picotm_begin

        struct txstack* stack = txstack_of_state_tx(&stack_state);

        /* Insert all ulong items to stack */
        {
            struct ulong_stack_item* beg = ulong_item;
            struct ulong_stack_item* end = ulong_item + picotm_arraylen(ulong_item);

            for (struct ulong_stack_item* pos = beg; pos < end; ++pos) {
                txstack_push_tx(stack, &pos->stack_entry);
            }
        }

        /* Compare top-most item with expected value; should be equal */
        {
            unsigned long value = STACK_BASE_VALUE +
                                  picotm_arraylen(ulong_item) - 1;

            struct txstack_entry* entry = txstack_top_tx(stack);

            const struct ulong_stack_item* item =
                ulong_stack_item_of_stack_entry(entry);

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

    txstack_state_clear_and_uninit_entries(&stack_state,
                                           uninit_txstack_entry_cb,
                                           NULL);
    txstack_state_uninit(&stack_state);
}

/*
 * Remove items from local stack.
 */

static void
txstack_test_3(unsigned int tid)
{
    struct txstack_state stack_state = TXSTACK_STATE_INITIALIZER(stack_state);

    struct ulong_stack_item ulong_item[STACK_MAXNITEMS];
    init_ulong_stack_items(picotm_arraybeg(ulong_item),
                           picotm_arrayend(ulong_item),
                           STACK_BASE_VALUE);

    picotm_begin

        struct txstack* stack = txstack_of_state_tx(&stack_state);

        /* Insert all ulong items to stack */
        {
            struct ulong_stack_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_stack_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_stack_item* pos = beg; pos < end; ++pos) {
                txstack_push_tx(stack, &pos->stack_entry);
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    picotm_begin

        struct txstack* stack = txstack_of_state_tx(&stack_state);

        if (txstack_empty_tx(stack)) {
            tap_error("condition failed: stack is non-empty before removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        if (txstack_size_tx(stack) != picotm_arraylen(ulong_item)) {
            tap_error("condition failed: stack contains all elements");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        /* Remove all items from stack. */
        {
            struct ulong_stack_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_stack_item* end = picotm_arrayend(ulong_item);

            unsigned long value = STACK_BASE_VALUE +
                                  picotm_arraylen(ulong_item);

            for (struct ulong_stack_item* pos = beg; pos < end; ++pos) {

                --value;

                struct txstack_entry* entry = txstack_top_tx(stack);
                txstack_pop_tx(stack);

                const struct ulong_stack_item* item =
                    ulong_stack_item_of_stack_entry(entry);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

        if (!txstack_empty_tx(stack)) {
            tap_error("condition failed: stack is empty after removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txstack_state_uninit(&stack_state);
}

/*
 * Push items onto shared stack.
 */

static void
txstack_test_4(unsigned int tid)
{
    struct ulong_stack_item* ulong_item[STACK_MAXNITEMS];

    {
        struct ulong_stack_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_stack_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_stack_item** pos = beg; pos != end; ++pos) {
            struct ulong_stack_item* item = safe_malloc(sizeof(*item));
            ulong_stack_item_init_with_value(item, tid);
            *pos = item;
        }
    }

    /* Insert all ulong items to stack */

    picotm_begin

        struct txstack* stack = txstack_of_state_tx(&g_stack_state);

        struct ulong_stack_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_stack_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_stack_item** pos = beg; pos != end; ++pos) {
            txstack_push_tx(stack, &(*pos)->stack_entry);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    safe_sched_yield();

    /* Compare values; consecutive items should have same value. */

    picotm_begin

        struct txstack* stack = txstack_of_state_tx(&g_stack_state);

        picotm_safe unsigned long value;

        for (size_t i = 0; i < picotm_arraylen(ulong_item); ++i) {

            struct txstack_entry* entry = txstack_top_tx(stack);

            struct ulong_stack_item* item =
                ulong_stack_item_of_stack_entry(entry);

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

            txstack_pop_tx(stack);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    /* Free items */

    {
        struct ulong_stack_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_stack_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_stack_item** pos = beg; pos != end; ++pos) {
            ulong_stack_item_uninit(*pos);
            free(*pos);
        }
    }
}

static void
txstack_test_4_pre(unsigned long nthreads, enum loop_mode loop,
                   enum boundary_type btype, unsigned long long bound)
{
    txstack_state_init(&g_stack_state);
}

static void
txstack_test_4_post(unsigned long nthreads, enum loop_mode loop,
                    enum boundary_type btype, unsigned long long bound)
{
    txstack_state_uninit(&g_stack_state);
}

static const struct test_func txstack_test[] = {
    {"txstack_test_1", txstack_test_1, NULL,               NULL},
    {"txstack_test_2", txstack_test_2, NULL,               NULL},
    {"txstack_test_3", txstack_test_3, NULL,               NULL},
    {"txstack_test_4", txstack_test_4, txstack_test_4_pre, txstack_test_4_post}
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
                       txstack_test, arraylen(txstack_test));
}
