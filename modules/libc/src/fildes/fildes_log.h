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
#include "fildes_event.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct fildes_log {

    unsigned long module;

    struct fildes_event* eventtab;
    size_t               eventtablen;
    size_t               eventtabsiz;
};

void
fildes_log_init(struct fildes_log* self, unsigned long module);

void
fildes_log_uninit(struct fildes_log* self);

static inline size_t
fildes_log_length(const struct fildes_log* self)
{
    return self->eventtablen;
}

static inline struct fildes_event*
fildes_log_at(const struct fildes_log* self, size_t i)
{
    return self->eventtab + i;
}

void
fildes_log_clear(struct fildes_log* self);

void
fildes_log_append(struct fildes_log* self, enum fildes_op op, int fildes,
                  int cookie, struct picotm_error* error);
