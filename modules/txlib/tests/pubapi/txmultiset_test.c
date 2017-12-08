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

#include "txmultiset_test.h"
#include <assert.h>
#include <picotm/picotm.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-ptr.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include <string.h>
#include "compat/static_assert.h"
#include "picotm/picotm-txmultiset.h"
#include "ptr.h"
#include "safeblk.h"
#include "safe_sched.h"
#include "safe_stdio.h"
#include "safe_stdlib.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

#define MULTISET_MAXNITEMS  16
#define MULTISET_BASE_VALUE 0

struct txmultiset_state g_multiset_state;

struct ulong_multiset_item {
    struct txmultiset_entry multiset_entry;

    unsigned long value;
};

static struct ulong_multiset_item*
ulong_multiset_item_of_multiset_entry(struct txmultiset_entry* multiset_entry)
{
    return picotm_containerof(multiset_entry,
                              struct ulong_multiset_item,
                              multiset_entry);
}

static const struct ulong_multiset_item*
ulong_multiset_item_of_const_multiset_entry(
    const struct txmultiset_entry* multiset_entry)
{
    return picotm_containerof(multiset_entry,
                              const struct ulong_multiset_item,
                              multiset_entry);
}

static void
ulong_multiset_item_init_with_value(struct ulong_multiset_item* self,
                                    unsigned long value)
{
    txmultiset_entry_init(&self->multiset_entry);
    self->value = value;
}

static void
ulong_multiset_item_uninit(struct ulong_multiset_item* self)
{
    txmultiset_entry_uninit(&self->multiset_entry);
}

static void
init_ulong_multiset_items(struct ulong_multiset_item* beg,
                    const struct ulong_multiset_item* end,
                          unsigned long base_value)
{
    for (struct ulong_multiset_item* pos = beg; pos < end; ++pos) {
        ulong_multiset_item_init_with_value(
            pos, base_value + (unsigned long)(pos - beg));
    }
}

static const void*
ulong_multiset_item_key(const struct ulong_multiset_item* self)
{
    return &self->value;
}

static int
ulong_compare(const unsigned long* lhs, const unsigned long* rhs)
{
    return (*rhs < *lhs) - (*lhs < *rhs);
}

static const void*
ulong_multiset_item_key_cb(struct txmultiset_entry* entry)
{
    return ulong_multiset_item_key(
        ulong_multiset_item_of_const_multiset_entry(entry));
}

static int
ulong_multiset_item_compare_cb(const void* lhs, const void* rhs)
{
    return ulong_compare(lhs, rhs);
}

static void
uninit_txmultiset_entry_cb(struct txmultiset_entry* entry, void* data)
{
    ulong_multiset_item_uninit(ulong_multiset_item_of_multiset_entry(entry));
}

/*
 * Declare multiset state and acquire multiset.
 */

static void
txmultiset_test_1(unsigned int tid)
{
    struct txmultiset_state multiset_state =
        TXMULTISET_STATE_INITIALIZER(multiset_state,
                                     ulong_multiset_item_key_cb,
                                     ulong_multiset_item_compare_cb);

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txmultiset_state_uninit(&multiset_state);
}

/*
 * Insert items into local multiset.
 */

static void
txmultiset_test_2(unsigned int tid)
{
    struct txmultiset_state multiset_state =
        TXMULTISET_STATE_INITIALIZER(multiset_state,
                                     ulong_multiset_item_key_cb,
                                     ulong_multiset_item_compare_cb);

    struct ulong_multiset_item ulong_item[MULTISET_MAXNITEMS];
    init_ulong_multiset_items(picotm_arraybeg(ulong_item),
                              picotm_arrayend(ulong_item),
                              MULTISET_BASE_VALUE);

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        /* Insert all ulong items to multiset */
        {
            struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_multiset_item* pos = beg; pos < end; ++pos) {
                txmultiset_insert_tx(multiset, &pos->multiset_entry);
            }
        }

        /* Compare multiset position with ulong value; should be equal. */
        {
            struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
            struct txmultiset_entry* end = txmultiset_end_tx(multiset);

            unsigned long value = MULTISET_BASE_VALUE;

            for (; beg != end; beg = txmultiset_entry_next_tx(beg), ++value) {

                const struct ulong_multiset_item* item =
                    ulong_multiset_item_of_multiset_entry(beg);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

        /* Compare multiset position with ulong value in reversed order;
         * should be equal. */
        {
            struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
            struct txmultiset_entry* end = txmultiset_end_tx(multiset);
            struct txmultiset_entry* pos = end;

            unsigned long value = MULTISET_BASE_VALUE +
                                  picotm_arraylen(ulong_item);

            while (pos != beg) {

                pos = txmultiset_entry_prev_tx(pos);
                --value;

                const struct ulong_multiset_item* item =
                    ulong_multiset_item_of_multiset_entry(pos);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txmultiset_state_clear_and_uninit_entries(&multiset_state,
                                              uninit_txmultiset_entry_cb,
                                              NULL);
    txmultiset_state_uninit(&multiset_state);
}

/*
 * Insert items into local multiset.
 */

static void
txmultiset_test_3(unsigned int tid)
{
    struct txmultiset_state multiset_state =
        TXMULTISET_STATE_INITIALIZER(multiset_state,
                                     ulong_multiset_item_key_cb,
                                     ulong_multiset_item_compare_cb);

    struct ulong_multiset_item ulong_item[MULTISET_MAXNITEMS];
    init_ulong_multiset_items(picotm_arraybeg(ulong_item),
                              picotm_arrayend(ulong_item),
                              MULTISET_BASE_VALUE);

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        /* Insert all ulong items to multiset in reversed order */
        {
            struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_multiset_item* pos = end; pos > beg;) {
                --pos;
                txmultiset_insert_tx(multiset, &pos->multiset_entry);
            }
        }

        /* Compare multiset position with ulong value; should be equal. */
        {
            struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
            struct txmultiset_entry* end = txmultiset_end_tx(multiset);

            unsigned long value = MULTISET_BASE_VALUE;

            for (; beg != end; beg = txmultiset_entry_next_tx(beg), ++value) {

                const struct ulong_multiset_item* item =
                    ulong_multiset_item_of_multiset_entry(beg);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

        /* Compare multiset position with ulong value in reversed order;
         * should be equal. */
        {
            struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
            struct txmultiset_entry* end = txmultiset_end_tx(multiset);
            struct txmultiset_entry* pos = end;

            unsigned long value = MULTISET_BASE_VALUE +
                                  picotm_arraylen(ulong_item);

            while (pos != beg) {

                pos = txmultiset_entry_prev_tx(pos);
                --value;

                const struct ulong_multiset_item* item =
                    ulong_multiset_item_of_multiset_entry(pos);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txmultiset_state_clear_and_uninit_entries(&multiset_state,
                                              uninit_txmultiset_entry_cb,
                                              NULL);
    txmultiset_state_uninit(&multiset_state);
}

/*
 * Insert items into local multiset.
 */

static void
txmultiset_test_4(unsigned int tid)
{
    struct txmultiset_state multiset_state =
        TXMULTISET_STATE_INITIALIZER(multiset_state,
                                     ulong_multiset_item_key_cb,
                                     ulong_multiset_item_compare_cb);

    struct ulong_multiset_item ulong_item[MULTISET_MAXNITEMS];
    init_ulong_multiset_items(picotm_arraybeg(ulong_item),
                              picotm_arrayend(ulong_item),
                              MULTISET_BASE_VALUE);

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        /* Insert all ulong items to multiset */
        {
            struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_multiset_item* pos = beg; pos < end; pos += 2) {
                txmultiset_insert_tx(multiset, &pos->multiset_entry);
            }

            for (struct ulong_multiset_item* pos = beg + 1; pos < end; pos += 2) {
                txmultiset_insert_tx(multiset, &pos->multiset_entry);
            }
        }

        /* Compare multiset position with ulong value; should be equal. */
        {
            struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
            struct txmultiset_entry* end = txmultiset_end_tx(multiset);

            unsigned long value = MULTISET_BASE_VALUE;

            for (; beg != end; beg = txmultiset_entry_next_tx(beg), ++value) {

                const struct ulong_multiset_item* item =
                    ulong_multiset_item_of_multiset_entry(beg);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

        /* Compare multiset position with ulong value in reversed order;
         * should be equal. */
        {
            struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
            struct txmultiset_entry* end = txmultiset_end_tx(multiset);
            struct txmultiset_entry* pos = end;

            unsigned long value = MULTISET_BASE_VALUE +
                                  picotm_arraylen(ulong_item);

            while (pos != beg) {

                pos = txmultiset_entry_prev_tx(pos);
                --value;

                const struct ulong_multiset_item* item =
                    ulong_multiset_item_of_multiset_entry(pos);

                if (item->value != value) {
                    tap_error("condition failed: value == item->value");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txmultiset_state_clear_and_uninit_entries(&multiset_state,
                                              uninit_txmultiset_entry_cb,
                                              NULL);
    txmultiset_state_uninit(&multiset_state);
}

/*
 * Remove items from local multiset.
 */

static void
txmultiset_test_5(unsigned int tid)
{
    struct txmultiset_state multiset_state =
        TXMULTISET_STATE_INITIALIZER(multiset_state,
                                     ulong_multiset_item_key_cb,
                                     ulong_multiset_item_compare_cb);

    struct ulong_multiset_item ulong_item[MULTISET_MAXNITEMS];
    init_ulong_multiset_items(picotm_arraybeg(ulong_item),
                              picotm_arrayend(ulong_item),
                              MULTISET_BASE_VALUE);

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        /* Insert all ulong items to multiset */
        {
            struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_multiset_item* pos = beg; pos < end; ++pos) {
                txmultiset_insert_tx(multiset, &pos->multiset_entry);
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        if (txmultiset_empty_tx(multiset)) {
            tap_error("condition failed: multiset is not empty before removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        /* Remove all items from multiset. */
        {
            struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_multiset_item* pos = beg; pos != end; ++pos) {
                txmultiset_erase_tx(multiset, &pos->multiset_entry);
            }
        }

        if (!txmultiset_empty_tx(multiset)) {
            tap_error("condition failed: multiset is empty after removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txmultiset_state_uninit(&multiset_state);
}

/*
 * Clear local multiset.
 */

static void
txmultiset_test_6(unsigned int tid)
{
    struct txmultiset_state multiset_state =
        TXMULTISET_STATE_INITIALIZER(multiset_state,
                                     ulong_multiset_item_key_cb,
                                     ulong_multiset_item_compare_cb);

    struct ulong_multiset_item ulong_item[MULTISET_MAXNITEMS];
    init_ulong_multiset_items(picotm_arraybeg(ulong_item),
                              picotm_arrayend(ulong_item),
                              MULTISET_BASE_VALUE);

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        /* Insert all ulong items to multiset */

        struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
        struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

        for (struct ulong_multiset_item* pos = beg; pos != end; ++pos) {
            txmultiset_insert_tx(multiset, &pos->multiset_entry);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        if (txmultiset_empty_tx(multiset)) {
            tap_error("condition failed: multiset is not empty before removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

        txmultiset_clear_tx(multiset);

        if (!txmultiset_empty_tx(multiset)) {
            tap_error("condition failed: multiset is empty after removal");
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txmultiset_state_uninit(&multiset_state);
}

/*
 * Find and count items in local multiset.
 */

static void
txmultiset_test_7(unsigned int tid)
{
    struct txmultiset_state multiset_state =
        TXMULTISET_STATE_INITIALIZER(multiset_state,
                                     ulong_multiset_item_key_cb,
                                     ulong_multiset_item_compare_cb);

    struct ulong_multiset_item ulong_item[MULTISET_MAXNITEMS];

    /* Initialize ulong items with value */

    static_assert(!(picotm_arraylen(ulong_item) % 4),
                  "Number of ulong items is not a multiple of 4");

    {
        struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
        struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

        for (struct ulong_multiset_item* pos = beg; pos != end; ++pos) {
            ulong_multiset_item_init_with_value(pos, (pos - beg) % 4);
        }
    }

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        /* Insert all ulong items into multiset */
        {
            struct ulong_multiset_item* beg = picotm_arraybeg(ulong_item);
            struct ulong_multiset_item* end = picotm_arrayend(ulong_item);

            for (struct ulong_multiset_item* pos = beg; pos != end; ++pos) {
                txmultiset_insert_tx(multiset, &pos->multiset_entry);
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    picotm_begin

        const size_t ncount = picotm_arraylen(ulong_item) / 4;

        const unsigned long nvalues = (unsigned long)picotm_arraylen(ulong_item) % 4;

        struct txmultiset* multiset = txmultiset_of_state_tx(&multiset_state);

        /* Find an entry for each value */
        {
            for (unsigned long value = 0; value < nvalues; ++value) {

                struct txmultiset_entry* entry = txmultiset_find_tx(multiset,
                                                                    &value);

                if (entry == txmultiset_end_tx(multiset)) {
                    tap_error("condition failed: entry != end");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

        /* Iterate over each value group's entries */
        {
            for (unsigned long value = 0; value < nvalues; ++value) {

                struct txmultiset_entry* lower =
                    txmultiset_lower_bound_tx(multiset, &value);
                struct txmultiset_entry* upper =
                    txmultiset_lower_bound_tx(multiset, &value);

                size_t count = 0;

                for (; lower != upper; lower = txmultiset_entry_next_tx(lower)) {

                    struct ulong_multiset_item* item =
                        ulong_multiset_item_of_multiset_entry(lower);

                    if (item->value != value) {
                        tap_error("condition failed: value == item->value");
                        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                        picotm_error_mark_as_non_recoverable(&error);
                        picotm_recover_from_error(&error);
                    }

                    ++count;
                }

                if (count != ncount) {
                    tap_error("condition failed: count == ncount");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

        /* Count each value group's entries */
        {
            for (unsigned long value = 0; value < nvalues; ++value) {

                size_t count = txmultiset_count_tx(multiset, &value);

                if (count != ncount) {
                    tap_error("condition failed: count == ncount");
                    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
                    picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
                    picotm_error_mark_as_non_recoverable(&error);
                    picotm_recover_from_error(&error);
                }
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    txmultiset_state_clear_and_uninit_entries(&multiset_state,
                                              uninit_txmultiset_entry_cb,
                                              NULL);
    txmultiset_state_uninit(&multiset_state);
}

/*
 * Insert values into shared multiset.
 */

static void
txmultiset_test_8(unsigned int tid)
{
    struct ulong_multiset_item* ulong_item[MULTISET_MAXNITEMS];

    {
        struct ulong_multiset_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_multiset_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_multiset_item** pos = beg; pos != end; ++pos) {
            struct ulong_multiset_item* item = safe_malloc(sizeof(*item));
            ulong_multiset_item_init_with_value(item, tid);
            *pos = item;
        }
    }

    /* Insert all ulong items to multiset */

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&g_multiset_state);

        struct ulong_multiset_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_multiset_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_multiset_item** pos = beg; pos != end; ++pos) {
            txmultiset_insert_tx(multiset, &(*pos)->multiset_entry);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    safe_sched_yield();

    /* Compare values; consecutive items should have same value. */

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&g_multiset_state);

        struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
        struct txmultiset_entry* end = txmultiset_end_tx(multiset);
        struct txmultiset_entry* pos = beg;

        unsigned long value;
        unsigned long i = 0;

        while (pos != end) {

            const struct ulong_multiset_item* item =
                ulong_multiset_item_of_multiset_entry(pos);

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

            pos = txmultiset_entry_next_tx(pos);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    safe_sched_yield();

    /* Remove all items with local TID from shared multiset. */

    picotm_begin

        struct txmultiset* multiset = txmultiset_of_state_tx(&g_multiset_state);

        struct txmultiset_entry* beg = txmultiset_begin_tx(multiset);
        struct txmultiset_entry* end = txmultiset_end_tx(multiset);
        struct txmultiset_entry* pos = beg;

        while (pos != end) {

            struct txmultiset_entry* next = txmultiset_entry_next_tx(pos);

            const struct ulong_multiset_item* item =
                ulong_multiset_item_of_multiset_entry(pos);

            if (item->value == tid) {
                txmultiset_erase_tx(multiset, pos);
                pos = txmultiset_begin_tx(multiset);
            } else {
                pos = next;
            }
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    /* Free items */

    {
        struct ulong_multiset_item** beg = picotm_arraybeg(ulong_item);
        struct ulong_multiset_item** end = picotm_arrayend(ulong_item);

        for (struct ulong_multiset_item** pos = beg; pos != end; ++pos) {
            ulong_multiset_item_uninit(*pos);
            free(*pos);
        }
    }
}

static void
txmultiset_test_8_pre(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound)
{
    txmultiset_state_init(&g_multiset_state,
                          ulong_multiset_item_key_cb,
                          ulong_multiset_item_compare_cb);
}

static void
txmultiset_test_8_post(unsigned long nthreads, enum loop_mode loop,
                   enum boundary_type btype, unsigned long long bound)
{
    txmultiset_state_uninit(&g_multiset_state);
}

const struct test_func txmultiset_test[] = {
    {"txmultiset_test_1", txmultiset_test_1, NULL,                  NULL},
    {"txmultiset_test_2", txmultiset_test_2, NULL,                  NULL},
    {"txmultiset_test_3", txmultiset_test_3, NULL,                  NULL},
    {"txmultiset_test_4", txmultiset_test_4, NULL,                  NULL},
    {"txmultiset_test_5", txmultiset_test_5, NULL,                  NULL},
    {"txmultiset_test_6", txmultiset_test_6, NULL,                  NULL},
    {"txmultiset_test_7", txmultiset_test_7, NULL,                  NULL},
    {"txmultiset_test_8", txmultiset_test_8, txmultiset_test_8_pre, txmultiset_test_8_post},
};

size_t
number_of_txmultiset_tests()
{
    return arraylen(txmultiset_test);
}
