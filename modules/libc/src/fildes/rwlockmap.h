/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <picotm/picotm-lib-rwlock.h>
#include <picotm/picotm-lib-shared-treemap.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

enum {
    RWLOCKMAP_PAGE_NBITS    = 9,
    RWLOCKMAP_PAGE_NENTRIES = 1ul << RWLOCKMAP_PAGE_NBITS,
    RWLOCKMAP_PAGE_MASK     = RWLOCKMAP_PAGE_NENTRIES - 1
};

struct rwlockmap_page {
    struct picotm_rwlock lock[RWLOCKMAP_PAGE_NENTRIES];
};

struct rwlockmap {
    struct picotm_shared_treemap pagemap;
};

void
rwlockmap_init(struct rwlockmap* self);

void
rwlockmap_uninit(struct rwlockmap* self);

struct rwlockmap_page*
rwlockmap_find_page(struct rwlockmap* self, unsigned long long record_offset,
                    struct picotm_error* error);
