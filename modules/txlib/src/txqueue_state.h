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

#include <stdbool.h>
#include "picotm/picotm-txqueue-state.h"

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct txqueue_entry*
txqueue_state_begin(struct txqueue_state* self);

struct txqueue_entry*
txqueue_state_end(struct txqueue_state* self);

bool
txqueue_state_is_empty(struct txqueue_state* self);

size_t
txqueue_state_size(struct txqueue_state* self);

struct txqueue_entry*
txqueue_state_front(struct txqueue_state* self);

struct txqueue_entry*
txqueue_state_back(struct txqueue_state* self);

void
txqueue_state_push_front(struct txqueue_state* self,
                         struct txqueue_entry* entry);

void
txqueue_state_push_back(struct txqueue_state* self,
                        struct txqueue_entry* entry);

void
txqueue_state_pop_front(struct txqueue_state* self);
