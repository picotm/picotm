/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "compiler.h"

/**
 * \ingroup group_modules
 * \file
 */

PICOTM_BEGIN_DECLS

/**
 * Computes the number of elements in a static C array.
 */
#define picotm_arraylen(_array) (sizeof(_array) / sizeof((_array)[0]))

/**
 * Returns the beginning of a static C array.
 */
#define picotm_arraybeg(_array) (_array)

/**
 * Returns the address after a static C array.
 */
#define picotm_arrayend(_array) ((_array) + (picotm_arraylen(_array)))

/**
 * Returns the address of the first element in a static C array.
 */
#define picotm_arrayfirst(_array)   (_array)

/**
 * Returns the address of the last element in a static C array.
 */
#define picotm_arraylast(_array)    (picotm_arrayend(_array) - 1)

PICOTM_END_DECLS
