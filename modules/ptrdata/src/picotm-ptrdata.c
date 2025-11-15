/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
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

#include "ptrdata.h"
#include "module.h"

PICOTM_EXPORT
void
ptr_set_shared_data(const void* ptr, const void* data,
                    struct picotm_error* error)
{
    ptrdata_module_set_shared_data(ptr, data, error);
}

PICOTM_EXPORT
_Bool
ptr_test_and_set_shared_data(const void* ptr, const void* current,
                             const void* data, struct picotm_error* error)
{
    return ptrdata_module_test_and_set_shared_data(ptr, current, data, error);
}

PICOTM_EXPORT
void
ptr_clear_shared_data(const void* ptr, struct picotm_error* error)
{
    ptrdata_module_clear_shared_data(ptr, error);
}

PICOTM_EXPORT
void*
ptr_get_shared_data(const void* ptr, struct picotm_error* error)
{
    return ptrdata_module_get_shared_data(ptr, error);
}

PICOTM_EXPORT
void
ptr_set_data(const void* ptr, const void* data, struct picotm_error* error)
{
    ptrdata_module_set_data(ptr, data, error);
}

PICOTM_EXPORT
void
ptr_clear_data(const void* ptr, struct picotm_error* error)
{
    ptrdata_module_clear_data(ptr, error);
}

PICOTM_EXPORT
void*
ptr_get_data(const void* ptr, struct picotm_error* error)
{
    return ptrdata_module_get_data(ptr, error);
}
