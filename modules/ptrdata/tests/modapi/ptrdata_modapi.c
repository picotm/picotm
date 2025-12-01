/*
 * picotm - A system-level transaction manager
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
#include "picotm/picotm-module.h"
#include "picotm/picotm-ptrdata.h"
#include <pthread.h>
#include "ptr.h"
#include "safeblk.h"
#include "sysenv.h"
#include "taputils.h"
#include "test.h"
#include "testhlp.h"

#if defined(__clang__)
#define NO_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__)
#define NO_OPTIMIZE __attribute__((optimize(0)))
#endif

/*
 * Module-like interface
 */

NO_OPTIMIZE
static void
set_ptr_data_tx(const void* ptr, const void* data)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        ptr_set_data(ptr, data, &error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

NO_OPTIMIZE
static void
clear_ptr_data_tx(const void* ptr)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        ptr_clear_data(ptr, &error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

NO_OPTIMIZE
static void*
get_ptr_data_tx(const void* ptr)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        void* data = ptr_get_data(ptr, &error);
        if (!picotm_error_is_set(&error)) {
            return data;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

NO_OPTIMIZE
static void
set_shared_ptr_data_tx(const void* ptr, const void* data)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        ptr_set_shared_data(ptr, data, &error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

NO_OPTIMIZE
static bool
test_and_set_shared_ptr_data_tx(const void* ptr, const void* current,
                                const void* data)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        bool succ = ptr_test_and_set_shared_data(ptr, current, data, &error);
        if (!picotm_error_is_set(&error)) {
            return succ;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

NO_OPTIMIZE
static void
clear_shared_ptr_data_tx(const void* ptr)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        ptr_clear_shared_data(ptr, &error);
        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

NO_OPTIMIZE
static void*
get_shared_ptr_data_tx(const void* ptr)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        void* data = ptr_get_shared_data(ptr, &error);
        if (!picotm_error_is_set(&error)) {
            return data;
        }
        picotm_recover_from_error(&error);
    } while (true);
}

/*
 * Test ptrdata_set_data()
 */

static void
test_ptrdata_set_data_tx(const void* ptr)
{
    set_ptr_data_tx(ptr, ptr);
}

static void
ptrdata_test_ptrdata_set_data(unsigned int tid)
{
    picotm_begin
        test_ptrdata_set_data_tx((void*)(uintptr_t)tid);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test ptrdata_get_data()
 */

static void
test_ptrdata_get_data_tx(const void* ptr)
{
    set_ptr_data_tx(ptr, ptr);
    const void* data = get_ptr_data_tx(ptr);

    if (data != ptr) {
        tap_error("%s, Incorrect data: got %p, expected %p.", __func__,
                  data, ptr);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }
}

static void
ptrdata_test_ptrdata_get_data(unsigned int tid)
{
    picotm_begin
        test_ptrdata_get_data_tx((void*)(uintptr_t)tid);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test ptrdata_clear_data()
 */

static void
test_ptrdata_clear_data_tx(const void* ptr)
{
    set_ptr_data_tx(ptr, ptr);

    const void* data = get_ptr_data_tx(ptr);
    if (data != ptr) {
        tap_error("%s, Incorrect data: got %p, expected %p.", __func__,
                  data, ptr);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }

    clear_ptr_data_tx(ptr);

    data = get_ptr_data_tx(ptr);
    if (data != NULL) {
        tap_error("%s, Incorrect data: got %p, expected NULL.", __func__,
                  data);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }
}

static void
ptrdata_test_ptrdata_clear_data(unsigned int tid)
{
    picotm_begin
        test_ptrdata_clear_data_tx((void*)(uintptr_t)tid);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test ptrdata_set_shared_data()
 */

static void
test_ptrdata_set_shared_data_tx(const void* ptr)
{
    set_shared_ptr_data_tx(ptr, ptr);
}

static void
ptrdata_test_ptrdata_set_shared_data(unsigned int tid)
{
    picotm_begin
        test_ptrdata_set_shared_data_tx((void*)(uintptr_t)tid);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test ptrdata_get_data()
 */

static void
test_ptrdata_get_shared_data_tx(const void* ptr)
{
    set_shared_ptr_data_tx(ptr, ptr);
    const void* data = get_shared_ptr_data_tx(ptr);

    if (data != ptr) {
        tap_error("%s, Incorrect data: got %p, expected %p.", __func__,
                  data, ptr);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }
}

static void
ptrdata_test_ptrdata_get_shared_data(unsigned int tid)
{
    picotm_begin
        test_ptrdata_get_shared_data_tx((void*)(uintptr_t)tid);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test ptrdata_clear_shared_data()
 */

static void
test_ptrdata_clear_shared_data_tx(const void* ptr)
{
    set_shared_ptr_data_tx(ptr, ptr);

    const void* data = get_shared_ptr_data_tx(ptr);
    if (data != ptr) {
        tap_error("%s, Incorrect data: got %p, expected %p.", __func__,
                  data, ptr);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }

    clear_shared_ptr_data_tx(ptr);

    data = get_shared_ptr_data_tx(ptr);
    if (data != NULL) {
        tap_error("%s, Incorrect data: got %p, expected NULL.", __func__,
                  data);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }
}

static void
ptrdata_test_ptrdata_clear_shared_data(unsigned int tid)
{
    picotm_begin
        test_ptrdata_clear_shared_data_tx((void*)(uintptr_t)tid);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

/*
 * Test ptrdata_test_and_set_shared_data()
 */

static void
test_ptrdata_test_and_set_shared_data_tx(const void* ptr)
{
    set_shared_ptr_data_tx(ptr, ptr);

    const void* data = get_shared_ptr_data_tx(ptr);
    if (data != ptr) {
        tap_error("%s, Incorrect data: got %p, expected %p.", __func__,
                  data, ptr);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }

    bool succ = test_and_set_shared_ptr_data_tx(ptr, ptr, NULL);
    if (!succ) {
        tap_error("%s, Test-and-set failed for %p.", __func__, ptr);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }

    data = get_shared_ptr_data_tx(ptr);
    if (data != NULL) {
        tap_error("%s, Incorrect data: got %p, expected NULL.", __func__,
                  data);
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;
        picotm_error_set_error_code(&error, PICOTM_GENERAL_ERROR);
        picotm_recover_from_error(&error);
    }
}

static void
ptrdata_test_ptrdata_test_and_set_shared_data(unsigned int tid)
{
    picotm_begin
        test_ptrdata_test_and_set_shared_data_tx((void*)(uintptr_t)tid);
    picotm_commit
        abort_transaction_on_error(__func__);
    picotm_end
}

static const struct test_func signal_test[] = {
    {"Test ptrdata_set_data()",   ptrdata_test_ptrdata_set_data,   NULL, NULL},
    {"Test ptrdata_get_data()",   ptrdata_test_ptrdata_get_data,   NULL, NULL},
    {"Test ptrdata_clear_data()", ptrdata_test_ptrdata_clear_data, NULL, NULL},
    {"Test ptrdata_set_shared_data()",   ptrdata_test_ptrdata_set_shared_data,   NULL, NULL},
    {"Test ptrdata_get_shared_data()",   ptrdata_test_ptrdata_get_shared_data,   NULL, NULL},
    {"Test ptrdata_clear_shared_data()", ptrdata_test_ptrdata_clear_shared_data, NULL, NULL},
    {"Test ptrdata_test_and_set_shared_data()", ptrdata_test_ptrdata_test_and_set_shared_data, NULL, NULL},
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
                       signal_test, arraylen(signal_test));
}
