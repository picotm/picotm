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

#include "txlist_test.h"
#include <picotm/picotm.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-ptr.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include <string.h>
#include "picotm/picotm-txlist.h"
#include "ptr.h"
#include "safeblk.h"
#include "safe_pthread.h"
#include "safe_stdio.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

#define LIST_MAXNITEMS  16
#define LIST_BASE_VALUE 0

struct txlist_state g_list_state;

struct ulong_list_item {
    struct txlist_entry list_entry;

    unsigned long value;
};

static struct ulong_list_item*
ulong_list_item_of_list_item(struct txlist_entry* list_entry)
{
    return picotm_containerof(list_entry, struct ulong_list_item, list_entry);
}

static void
init_ulong_list_items(struct ulong_list_item* beg,
                const struct ulong_list_item* end, unsigned long base_value)
{
    for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
        txlist_entry_init(&pos->list_entry);
        pos->value = base_value + (unsigned long)(pos - beg);
    }
}

static void
init_ulong_list_items_with_value(struct ulong_list_item* beg,
                           const struct ulong_list_item* end,
                                 unsigned long tid)
{
    for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
        txlist_entry_init(&pos->list_entry);
        pos->value = tid;
    }
}

/*
 * Declare list state and acquire list.
 */

static void
txlist_test_1(unsigned int tid)
{
    struct txlist_state list_state = TXLIST_STATE_INITIALIZER(list_state);

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/*
 * Insert items at the end of local list.
 */

static void
txlist_test_2(unsigned int tid)
{
    struct txlist_state list_state = TXLIST_STATE_INITIALIZER(list_state);

    struct ulong_list_item ulong_item[LIST_MAXNITEMS];
    init_ulong_list_items(ulong_item,
                          ulong_item + picotm_arraylen(ulong_item),
                          LIST_BASE_VALUE);

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

        /* Insert all ulong items to list */
        {
            struct ulong_list_item* beg = ulong_item;
            struct ulong_list_item* end = ulong_item + picotm_arraylen(ulong_item);
            struct txlist_entry* position = txlist_end_tx(list);

            for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
                txlist_insert_tx(list, &pos->list_entry, position);
                position = &pos->list_entry;
            }
        }

        /* Compare reversed list position with ulong value; should be equal. */
        {
            struct txlist_entry* beg = txlist_begin_tx(list);
            struct txlist_entry* end = txlist_end_tx(list);
            struct txlist_entry* pos = end;

            unsigned long value = LIST_BASE_VALUE;

            while (pos != beg) {

                pos = txlist_entry_prev_tx(pos);

                const struct ulong_list_item* item =
                    ulong_list_item_of_list_item(pos);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }

                ++value;
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/*
 * Remove items from local list.
 */

static void
txlist_test_3(unsigned int tid)
{
    struct txlist_state list_state = TXLIST_STATE_INITIALIZER(list_state);

    struct ulong_list_item ulong_item[LIST_MAXNITEMS];
    init_ulong_list_items(ulong_item,
                          ulong_item + picotm_arraylen(ulong_item),
                          LIST_BASE_VALUE);

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

        /* Insert all ulong items to list */
        {
            struct ulong_list_item* beg = ulong_item;
            struct ulong_list_item* end = ulong_item + picotm_arraylen(ulong_item);
            struct txlist_entry* position = txlist_end_tx(list);

            for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
                txlist_insert_tx(list, &pos->list_entry, position);
                position = &pos->list_entry;
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

        if (txlist_empty_tx(list)) {
            tap_error("condition failed: list is not empty before removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        /* Remove all items from list. */
        {
            struct ulong_list_item* beg = ulong_item;
            struct ulong_list_item* end = ulong_item + picotm_arraylen(ulong_item);

            for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
                txlist_erase_tx(list, &pos->list_entry);
            }
        }

        if (!txlist_empty_tx(list)) {
            tap_error("condition failed: list is empty after removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/*
 * Append items to local list.
 */

static void
txlist_test_4(unsigned int tid)
{
    struct txlist_state list_state = TXLIST_STATE_INITIALIZER(list_state);

    struct ulong_list_item ulong_item[LIST_MAXNITEMS];
    init_ulong_list_items(ulong_item,
                          ulong_item + picotm_arraylen(ulong_item),
                          LIST_BASE_VALUE);

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

        /* Append all ulong items to list */
        {
            struct ulong_list_item* beg = ulong_item;
            struct ulong_list_item* end = ulong_item + picotm_arraylen(ulong_item);

            for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
                txlist_push_back_tx(list, &pos->list_entry);
            }
        }

        /* Compare list position with ulong value; should be equal. */
        {
            struct txlist_entry* beg = txlist_begin_tx(list);
            const struct txlist_entry* end = txlist_end_tx(list);

            unsigned long value = LIST_BASE_VALUE;

            while (beg != end) {

                const struct ulong_list_item* item =
                    ulong_list_item_of_list_item(beg);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }

                ++value;
                beg = txlist_entry_next_tx(beg);
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/*
 * Prepend items to local list.
 */

static void
txlist_test_5(unsigned int tid)
{
    struct txlist_state list_state = TXLIST_STATE_INITIALIZER(list_state);

    struct ulong_list_item ulong_item[LIST_MAXNITEMS];
    init_ulong_list_items(ulong_item,
                          ulong_item + picotm_arraylen(ulong_item),
                          LIST_BASE_VALUE);

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

        /* Prepend all ulong items to list */
        {
            struct ulong_list_item* beg = ulong_item;
            struct ulong_list_item* end = ulong_item + picotm_arraylen(ulong_item);

            for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
                txlist_push_front_tx(list, &pos->list_entry);
            }
        }

        /* Compare reversed list position with ulong value; should be equal. */
        {
            struct txlist_entry* beg = txlist_begin_tx(list);
            struct txlist_entry* end = txlist_end_tx(list);
            struct txlist_entry* pos = end;

            unsigned long value = LIST_BASE_VALUE;

            while (pos != beg) {

                pos = txlist_entry_prev_tx(pos);

                const struct ulong_list_item* item =
                    ulong_list_item_of_list_item(pos);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }

                ++value;
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/*
 * Clear local list.
 */

static void
txlist_test_6(unsigned int tid)
{
    struct txlist_state list_state = TXLIST_STATE_INITIALIZER(list_state);

    struct ulong_list_item ulong_item[LIST_MAXNITEMS];
    init_ulong_list_items(ulong_item,
                          ulong_item + picotm_arraylen(ulong_item),
                          LIST_BASE_VALUE);

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

        /* Insert all ulong items to list */
        {
            struct ulong_list_item* beg = ulong_item;
            struct ulong_list_item* end = ulong_item + picotm_arraylen(ulong_item);
            struct txlist_entry* position = txlist_end_tx(list);

            for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
                txlist_insert_tx(list, &pos->list_entry, position);
                position = &pos->list_entry;
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&list_state);

        if (txlist_empty_tx(list)) {
            tap_error("condition failed: list is not empty before removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        txlist_clear_tx(list);

        if (!txlist_empty_tx(list)) {
            tap_error("condition failed: list is empty after removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/*
 * Insert items into shared list.
 */

static void
txlist_test_7(unsigned int tid)
{
    struct ulong_list_item ulong_item[LIST_MAXNITEMS];
    init_ulong_list_items_with_value(ulong_item,
                                     ulong_item + picotm_arraylen(ulong_item),
                                     tid);

    /* Insert all ulong items to list */

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&g_list_state);

        struct ulong_list_item* beg = ulong_item;
        struct ulong_list_item* end = ulong_item + picotm_arraylen(ulong_item);
        struct txlist_entry* position = txlist_end_tx(list);

        for (struct ulong_list_item* pos = beg; pos < end; ++pos) {
            txlist_insert_tx(list, &pos->list_entry, position);
            position = &pos->list_entry;
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    safe_pthread_yield();

    /* Compare values; consecutive items should have same value. */

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&g_list_state);

        struct txlist_entry* beg = txlist_begin_tx(list);
        struct txlist_entry* end = txlist_end_tx(list);
        struct txlist_entry* pos = beg;

        unsigned long value;
        unsigned long i = 0;

        while (pos != end) {

            const struct ulong_list_item* item =
                ulong_list_item_of_list_item(pos);

            if (!i) {
                i = picotm_arraylen(ulong_item);
                value = item->value;
            } else if (item->value != value) {
                tap_error("condition failed: value == item->value");
                struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
            }

            --i;

            pos = txlist_entry_next_tx(pos);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    safe_pthread_yield();

    /* Remove all items with local TID from shared list. */

    picotm_begin

        struct txlist* list = txlist_of_state_tx(&g_list_state);

        struct txlist_entry* beg = txlist_begin_tx(list);
        struct txlist_entry* end = txlist_end_tx(list);
        struct txlist_entry* pos = beg;

        while (pos != end) {

            struct txlist_entry* next = txlist_entry_next_tx(pos);

            const struct ulong_list_item* item =
                ulong_list_item_of_list_item(pos);

            if (item->value == tid) {
                txlist_erase_tx(list, pos);
            }

            pos = next;
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static void
txlist_test_7_pre(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound)
{
    txlist_state_init(&g_list_state);
}

static void
txlist_test_7_post(unsigned long nthreads, enum loop_mode loop,
                   enum boundary_type btype, unsigned long long bound)
{
    txlist_state_uninit(&g_list_state);
}

const struct test_func txlist_test[] = {
    {"txlist_test_1", txlist_test_1, NULL,              NULL},
    {"txlist_test_2", txlist_test_2, NULL,              NULL},
    {"txlist_test_3", txlist_test_3, NULL,              NULL},
    {"txlist_test_4", txlist_test_4, NULL,              NULL},
    {"txlist_test_5", txlist_test_5, NULL,              NULL},
    {"txlist_test_6", txlist_test_6, NULL,              NULL},
    {"txlist_test_7", txlist_test_7, txlist_test_7_pre, txlist_test_7_post}
};

size_t
number_of_txlist_tests()
{
    return arraylen(txlist_test);
}
