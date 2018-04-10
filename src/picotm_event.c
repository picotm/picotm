/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "picotm_event.h"
#include "picotm/picotm-error.h"

void
picotm_events_foreach1(struct picotm_event* beg,
                 const struct picotm_event* end,
                       void* data,
                       void (*call)(struct picotm_event*, void*,
                                    struct picotm_error*),
                       struct picotm_error* error)
{
    while ((beg < end) && !picotm_error_is_set(error)) {
        call(beg, data, error);
        ++beg;
    }
}

void
picotm_events_rev_foreach1(struct picotm_event* beg,
                     const struct picotm_event* end,
                           void* data,
                           void (*call)(struct picotm_event*, void*,
                                        struct picotm_error*),
                           struct picotm_error* error)
{
    struct picotm_event* pos = (struct picotm_event*)end;

    while ((beg < pos) && !picotm_error_is_set(error)) {
        --pos;
        call(pos, data, error);
    }
}
