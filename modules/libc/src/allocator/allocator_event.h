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

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \ingroup libc_impl
 * \ingroup libc_impl_allocator
 * \file
 * \endcond
 */

struct allocator_tx;

/**
 * \brief Opcodes for allocator events.
 */
enum allocator_op {
    /** \brief Represents an allocator free() operation. */
    ALLOCATOR_OP_FREE = 0,
    /** \brief Represents an allocator malloc() operation. */
    ALLOCATOR_OP_MALLOC,
    /** \brief Represents an allocator posix_memalign() operation. */
    ALLOCATOR_OP_POSIX_MEMALIGN,
    /** \brief The number of allocator operations. */
    LAST_ALLOCATOR_OP
};

/**
 * \brief Represents an allocator event.
 */
struct allocator_event {
    void* ptr;
};
