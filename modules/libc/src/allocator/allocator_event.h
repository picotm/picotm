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