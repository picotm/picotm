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

#include <picotm/picotm-lib-rwstate.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/queue.h>
#include "block.h"

struct tm_vmem;

enum {
    TM_PAGE_FLAG_WRITE_THROUGH = 1 << 0,
    TM_PAGE_FLAG_DISCARDED     = 1 << 1
};

/**
 * |struct tm_page| represents the transaction-local state of a block
 * of memory. The global counterpart is called |struct tm_frame|.
 */
struct tm_page {
    /** Block index and flag bits */
    uintptr_t flags;

    /** Lock state wrt. to frame lock */
    struct picotm_rwstate rwstate;

    /** Transaction-local buffer */
    uint8_t buf[TM_BLOCK_SIZE];

    /** Entry into allocator lists */
    SLIST_ENTRY(tm_page) list;
};

void
tm_page_init(struct tm_page* page, size_t block_index);

void
tm_page_uninit(struct tm_page* page);

static inline size_t
tm_page_block_index(const struct tm_page* page)
{
    return page->flags >> TM_BLOCK_SIZE_BITS;
}

static inline uintptr_t
tm_page_address(const struct tm_page* page)
{
    return tm_page_block_index(page) * TM_BLOCK_SIZE;
}

void*
tm_page_buffer(struct tm_page* page);

void
tm_page_ld(struct tm_page* page, struct tm_vmem* vmem,
           struct picotm_error* error);

void
tm_page_st(struct tm_page* page, struct tm_vmem* vmem,
           struct picotm_error* error);

void
tm_page_xchg(struct tm_page* page, struct tm_vmem* vmem,
             struct picotm_error* error);

static inline bool
tm_page_has_locked_frame(const struct tm_page* page)
{
    return picotm_rwstate_get_status(&page->rwstate) != PICOTM_RWSTATE_UNLOCKED;
}

static inline bool
tm_page_has_rdlocked_frame(const struct tm_page* page)
{
    /* writer lock counts as implicit reader lock */
    return picotm_rwstate_get_status(&page->rwstate) != PICOTM_RWSTATE_UNLOCKED;
}

static inline bool
tm_page_has_wrlocked_frame(struct tm_page* page)
{
    return picotm_rwstate_get_status(&page->rwstate) == PICOTM_RWSTATE_WRLOCKED;
}

void
tm_page_try_rdlock_frame(struct tm_page* page, struct tm_vmem* vmem,
                         struct picotm_error* error);

void
tm_page_try_wrlock_frame(struct tm_page* page, struct tm_vmem* vmem,
                         struct picotm_error* error);

void
tm_page_unlock_frame(struct tm_page* page, struct tm_vmem* vmem,
                     struct picotm_error* error);
