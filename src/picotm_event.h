/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <stdint.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * \brief Represents an event in a transaction's event log.
 */
struct picotm_event {
    /** \brief The module id. */
    uint16_t module;
    /** \brief The event's head data field. */
    uint16_t head;

    /* Possibly padding bytes here */

    /** \brief A pointer to the event's tail data, or the tail data itself. */
    uintptr_t tail;
};

/**
 * Static initializer for `struct picotm_event`.
 * \param   _module The instance's module value.
 * \param   _head   The instance's head data.
 * \param   _tail   The instance's tail data.
 */
#define PICOTM_EVENT_INITIALIZER(_module, _head, _tail) \
    {                                                   \
        (_module),                                      \
        (_head),                                        \
        (_tail)                                         \
    }

void
picotm_events_foreach1(struct picotm_event* beg,
                 const struct picotm_event* end,
                       void* data,
                       void (*call)(struct picotm_event*, void*,
                                    struct picotm_error*),
                       struct picotm_error* error);

void
picotm_events_rev_foreach1(struct picotm_event* beg,
                     const struct picotm_event* end,
                           void* data,
                           void (*call)(struct picotm_event*, void*,
                                        struct picotm_error*),
                           struct picotm_error* error);
