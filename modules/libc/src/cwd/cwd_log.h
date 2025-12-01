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
#include "cwd_event.h"

/**
 * \cond impl || libc_impl || libc_impl_cwd
 * \ingroup libc_impl
 * \ingroup libc_impl_cwd
 * \file
 * \endcond
 */

struct picotm_error;

struct cwd_log {

    unsigned long module;

    struct cwd_event* eventtab;
    size_t            eventtablen;
    size_t            eventtabsiz;
};

void
cwd_log_init(struct cwd_log* self, unsigned long module);

void
cwd_log_uninit(struct cwd_log* self);

static inline size_t
cwd_log_length(const struct cwd_log* self)
{
    return self->eventtablen;
}

static inline struct cwd_event*
cwd_log_at(const struct cwd_log* self, size_t i)
{
    return self->eventtab + i;
}

void
cwd_log_clear(struct cwd_log* self);

void
cwd_log_append(struct cwd_log* self, enum cwd_op op, char* alloced,
               struct picotm_error* error);
