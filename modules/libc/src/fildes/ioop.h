/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

struct ioop {
    off_t  off;
    size_t nbyte;
    size_t bufoff;
};

void
ioop_init(struct ioop* self, off_t offset, size_t nbyte, size_t bufoff);

void
ioop_uninit(struct ioop* self);

/**
 * Reads an ioop into a buffer.
 * \param   self    The I/O operation.
 * \param   buf     The output buffer.
 * \param   nbyte   The output-buffer length.
 * \param   offset  The read offset.
 * \param   iobuf   The ioop data buffer, can be NULL if no data has been written.
 * \returns The number of bytes read.
 */
ssize_t
ioop_read(const struct ioop* self, void* buf, size_t nbyte, off_t offset,
          const void* iobuf);
