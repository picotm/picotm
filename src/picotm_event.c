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
