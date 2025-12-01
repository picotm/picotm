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

#include <stddef.h>
#include "locale_event.h"

/**
 * \cond impl || libc_impl || libc_impl_lcoale
 * \ingroup libc_impl
 * \ingroup libc_impl_locale
 * \file
 * \endcond
 */

struct picotm_error;

struct locale_log {

    unsigned long module;

    struct locale_event* eventtab;
    size_t               eventtablen;
    size_t               eventtabsiz;
};

void
locale_log_init(struct locale_log* self, unsigned long module);

void
locale_log_uninit(struct locale_log* self);

static inline size_t
locale_log_length(const struct locale_log* self)
{
    return self->eventtablen;
}

static inline struct locale_event*
locale_log_at(const struct locale_log* self, size_t i)
{
    return self->eventtab + i;
}

void
locale_log_clear(struct locale_log* self);

void
locale_log_append(struct locale_log* self, enum locale_op op,
                  struct locale_event* arg, struct picotm_error* error);
