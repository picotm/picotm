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
