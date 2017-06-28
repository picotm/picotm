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
 * Returns the address of the element at the specified index in a
 * static C array.
 */
#define picotm_arrayat(_array, _i)  (picotm_arraybeg(_array) + (_i))

/**
 * Returns the address after a static C array.
 */
#define picotm_arrayend(_array) \
    picotm_arrayat(_array, picotm_arraylen(_array))

/**
 * Returns the address of the first element in a static C array.
 */
#define picotm_arrayfirst(_array)   (_array)

/**
 * Returns the address of the last element in a static C array.
 */
#define picotm_arraylast(_array)    (picotm_arrayend(_array) - 1)

PICOTM_END_DECLS
