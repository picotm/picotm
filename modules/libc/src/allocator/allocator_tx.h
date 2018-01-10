/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <stddef.h>
#include <stdint.h>

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \ingroup libc_impl
 * \ingroup libc_impl_allocator
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * A transaction on the memory allocation.
 */
struct allocator_tx {
    unsigned long module;

    void** ptrtab;
    size_t ptrtablen;
    size_t ptrtabsiz;
};

void
allocator_tx_init(struct allocator_tx* self, unsigned long module);

void
allocator_tx_uninit(struct allocator_tx* self);

#if defined(HAVE_POSIX_MEMALIGN) && HAVE_POSIX_MEMALIGN
void
allocator_tx_exec_posix_memalign(struct allocator_tx* self, void** memptr,
                                 size_t alignment, size_t size,
                                 struct picotm_error* error);
#endif

void*
allocator_tx_exec_malloc(struct allocator_tx* self, size_t size,
                         struct picotm_error* error);

void
allocator_tx_exec_free(struct allocator_tx* self, void* ptr,
                       struct picotm_error* error);

void
allocator_tx_apply_event(struct allocator_tx* self,
                         unsigned short op, uintptr_t cookie,
                         struct picotm_error* error);

void
allocator_tx_undo_event(struct allocator_tx* self,
                        unsigned short op, uintptr_t cookie,
                        struct picotm_error* error);

void
allocator_tx_finish(struct allocator_tx* self);
