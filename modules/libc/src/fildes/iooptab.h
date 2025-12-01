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

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct ioop;
struct picotm_error;

unsigned long
iooptab_append(struct ioop** restrict tab, size_t* nelems, size_t* siz,
               size_t nbyte, off_t offset, size_t bufoff,
               struct picotm_error* error);

void
iooptab_clear(struct ioop** tab, size_t* nelems);

ssize_t
iooptab_read(struct ioop* tab, size_t nelems, void* buf, size_t nbyte,
             off_t offset, void* iobuf);

void
iooptab_sort(const struct ioop* tab, size_t nelems, struct ioop** sorted,
             struct picotm_error* error);
