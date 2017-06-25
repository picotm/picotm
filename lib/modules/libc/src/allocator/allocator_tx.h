/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#pragma once

#include <stddef.h>

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \ingroup libc_impl
 * \ingroup libc_impl_allocator
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_event;

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

void
allocator_tx_exec_posix_memalign(struct allocator_tx* self, void** memptr,
                                 size_t alignment, size_t size,
                                 struct picotm_error* error);

void
allocator_tx_exec_free(struct allocator_tx* self, void* ptr,
                       struct picotm_error* error);

void
allocator_tx_apply_event(struct allocator_tx* self,
                         const struct picotm_event* event,
                         struct picotm_error* error);

void
allocator_tx_undo_event(struct allocator_tx* self,
                        const struct picotm_event* event,
                        struct picotm_error* error);

void
allocator_tx_finish(struct allocator_tx* self);
