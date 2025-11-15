/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-lib-slist.h"
#include <stdbool.h>
#include <stdint.h>
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

    /** Bitmap of the valid fields in buf. */
    uint8_t buf_bits;

    /** Entry into allocator lists */
    struct picotm_slist list;
};

static inline struct tm_page*
tm_page_of_slist(struct picotm_slist* item)
{
    return picotm_containerof(item, struct tm_page, list);
}

static inline const struct tm_page*
tm_page_of_const_slist(const struct picotm_slist* item)
{
    return picotm_containerof(item, const struct tm_page, list);
}

void
tm_page_init(struct tm_page* page, size_t block_index);

void
tm_page_uninit(struct tm_page* page);

bool
tm_page_is_complete(const struct tm_page* self);

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
tm_page_ld(struct tm_page* page, unsigned long bits, struct tm_vmem* vmem,
           struct picotm_error* error);

bool
tm_page_ld_c(struct tm_page* page, unsigned long bits, int c, struct tm_vmem* vmem,
             struct picotm_error* error);

void
tm_page_st(struct tm_page* page, unsigned long bits, struct tm_vmem* vmem,
           struct picotm_error* error);

void
tm_page_xchg(struct tm_page* page, unsigned long bits, struct tm_vmem* vmem,
             struct picotm_error* error);
bool
tm_page_xchg_c(struct tm_page* page, unsigned long bits, int c, struct tm_vmem* vmem,
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
