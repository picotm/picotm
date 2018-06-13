/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/config/picotm-ptrdata-config.h"
#include "picotm/compiler.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_ptrdata
 * \file
 *
 * \brief Public interfaces of picotm's pointer-data module.
 */

struct picotm_error;

PICOTM_NOTHROW
/**
 * \ingroup group_ptrdata
 * Sets shared data for an address.
 * \param       ptr     The address
 * \param       data    The address' data
 * \param[out]  error   Returns an error to the caller.
 */
void
ptr_set_shared_data(const void* ptr, const void* data,
                    struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_ptrdata
 * Sets shared data for an address if the address has the expected
 * data value set.
 * \param       ptr     The address
 * \param       current The address' expected data
 * \param       data    The address' new data
 * \param[out]  error   Returns an error to the caller.
 * \returns Returns true on success, or false otherwise.
 */
_Bool
ptr_test_and_set_shared_data(const void* ptr, const void* current,
                             const void* data, struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_ptrdata
 * Removes shared data from an address.
 * \param       ptr     The address
 * \param[out]  error   Returns an error to the caller.
 */
void
ptr_clear_shared_data(const void* ptr, struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_ptrdata
 * Returns shared data of an address.
 * \param       ptr     The address
 * \param[out]  error   Returns an error to the caller.
 */
void*
ptr_get_shared_data(const void* ptr, struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_ptrdata
 * Sets thread-local data for an address.
 * \param       ptr     The address
 * \param       data    The address' data
 * \param[out]  error   Returns an error to the caller.
 */
void
ptr_set_data(const void* ptr, const void* data, struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_ptrdata
 * Removes thread-local data from an address.
 * \param       ptr     The address
 * \param[out]  error   Returns an error to the caller.
 */
void
ptr_clear_data(const void* ptr, struct picotm_error* error);

PICOTM_NOTHROW
/**
 * \ingroup group_ptrdata
 * Returns thread-local data of an address.
 * \param       ptr     The address
 * \param[out]  error   Returns an error to the caller.
 */
void*
ptr_get_data(const void* ptr, struct picotm_error* error);

PICOTM_END_DECLS

/**
 * \defgroup group_ptrdata The Pointer-Data Module
 *
 * \brief The pointer-data module associates data items with arbitrary
 *        memory locations. The data can be stored globally or local
 *        to the thread.
 *
 * The pointer-data module associates data items with arbitrary memory
 * locations. The data can be stored globally or local to the thread. The
 * pointer-data module is a helper for other module and not to be used
 * directly in application transactions.
 *
 * Thread-local data is associated with an address with `ptr_set_data()`.
 *
 * ~~~{.c}
 *  void* ptr = create_a_ptr_data_structure();
 *
 *  int* data = malloc(sizeof(int));
 *  *data = 0x1234;
 *
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *  ptr_set_data(ptr, data, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 * ~~~
 *
 * Once set, the data can be queried with `ptr_get_data()`.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *  int* data = ptr_get_data(ptr, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 * ~~~
 *
 * By default, the value returned by `ptr_get_data()`is NULL for address
 * without data. Data can be cleared with a call to `ptr_clear_data()`.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  int* data = ptr_get_data(ptr, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 *  ptr_clear_data(ptr, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 *
 *  free(data);
 * ~~~
 *
 * This call resets the address' data to NULL. It's equivalent to
 * `ptr_set_data()` with a data value of NULL, but faster if address
 * has no data already associated with it. The function only clears
 * the entry, the associated data itself has to be cleaned up by the
 * caller.
 *
 * For globally shared data, a similar interface exists. Data is globally
 * associated with an address with `ptr_set_shared_data()`, retrieved with
 * `ptr_get_shared_data()`, and cleared with `ptr_clear_shared_data()`.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  void* ptr = create_a_ptr_data_structure();
 *
 *  int* data = malloc(sizeof(int));
 *  *data = 0x1234;
 *
 *  ptr_set_shared_data(ptr, data, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 *  int* data = ptr_get_shared_data(ptr, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 *  ptr_clear_shared_data(ptr, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 *
 *  free(data);
 * ~~~
 *
 * All shared operations are atomic. If multiple threads try to set an
 * address' data concurrently, exactly one will succeed. The atomic
 * test-and-set function `ptr_test_and_set_shared_data()` is available.
 * The example below replaces an address data only if the current data
 * is NULL.
 *
 * ~~~{.c}
 *  struct picotm_error error = PICOTM_ERROR_INITIALIZER;
 *
 *  void* ptr = create_a_ptr_data_structure();
 *
 *  int* data = malloc(sizeof(int));
 *  *data = 0x1234;
 *
 *  succ = ptr_test_and_set_shared_data(ptr, NULL, data, &error);
 *  if (picotm_error_is_set(&error)) {
 *      // handle error
 *  }
 *  if (succ) {
 *      // NULL has been replaced by the value of data
 *  } else {
 *      // the previous non-NULL value remained in place
 *  }
 * ~~~
 *
 * The pointer-data module does not clean up data value at the end of a
 * thread of the application. Its using modules are responsible for this.
 */
