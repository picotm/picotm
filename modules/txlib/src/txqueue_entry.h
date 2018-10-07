/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
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

void
txqueue_entry_init_head(struct txqueue_entry* self);

void
txqueue_entry_uninit_head(struct txqueue_entry* self);

struct txqueue_entry*
txqueue_entry_next(const struct txqueue_entry* self);

struct txqueue_entry*
txqueue_entry_prev(const struct txqueue_entry* self);

bool
txqueue_entry_is_enqueued(const struct txqueue_entry* self);

void
txqueue_entry_insert(struct txqueue_entry* self, struct txqueue_entry* next);

void
txqueue_entry_erase(struct txqueue_entry* self);

/*
 * Queue-entry helpers
 */

size_t
txqueue_entry_distance(const struct txqueue_entry* beg,
                       const struct txqueue_entry* end);
