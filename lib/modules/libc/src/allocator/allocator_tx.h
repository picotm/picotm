/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
                         const struct picotm_event* event, size_t nevents,
                         struct picotm_error* error);

void
allocator_tx_undo_event(struct allocator_tx* self,
                        const struct picotm_event* event, size_t nevents,
                        struct picotm_error* error);

void
allocator_tx_finish(struct allocator_tx* self);
