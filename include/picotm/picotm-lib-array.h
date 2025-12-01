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

#pragma once

#include "compiler.h"

/**
 * \ingroup group_lib
 * \file
 */

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_lib
 * Computes the number of elements in a static C array.
 */
#define picotm_arraylen(_array) (sizeof(_array) / sizeof((_array)[0]))

/**
 * \ingroup group_lib
 * Returns the beginning of a static C array.
 */
#define picotm_arraybeg(_array) (_array)

/**
 * \ingroup group_lib
 * Returns the address of the element at the specified index in a
 * static C array.
 */
#define picotm_arrayat(_array, _i)  (picotm_arraybeg(_array) + (_i))

/**
 * \ingroup group_lib
 * Returns the address after a static C array.
 */
#define picotm_arrayend(_array) \
    picotm_arrayat(_array, picotm_arraylen(_array))

/**
 * \ingroup group_lib
 * Returns the address of the first element in a static C array.
 */
#define picotm_arrayfirst(_array)   (_array)

/**
 * \ingroup group_lib
 * Returns the address of the last element in a static C array.
 */
#define picotm_arraylast(_array)    (picotm_arrayend(_array) - 1)

PICOTM_END_DECLS
