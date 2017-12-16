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

#include "vmem_tx.h"
#include <picotm/picotm-error.h>
#include <stdlib.h>
#include <string.h>
#include "picotm/picotm-tm.h"
#include "page.h"

void
tm_vmem_tx_init(struct tm_vmem_tx* vmem_tx, struct tm_vmem* vmem,
                unsigned long module)
{
    vmem_tx->vmem = vmem;
    vmem_tx->module = module;

    SLIST_INIT(&vmem_tx->active_pages);
    SLIST_INIT(&vmem_tx->alloced_pages);
}

static void
free_page_list(struct tm_page_list* pages)
{
    while (!SLIST_EMPTY(pages)) {
        struct tm_page* page = SLIST_FIRST(pages);
        SLIST_REMOVE_HEAD(pages, list);
        free(page);
    }
}

void
tm_vmem_tx_release(struct tm_vmem_tx* vmem_tx)
{
    free_page_list(&vmem_tx->active_pages);
    free_page_list(&vmem_tx->alloced_pages);
}

static struct tm_page*
alloc_page(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    struct tm_page* page;

    if (SLIST_EMPTY(&vmem_tx->alloced_pages)) {
        page = malloc(sizeof(*page));
        if (!page) {
            picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
            return NULL;
        }
    } else {
        page = SLIST_FIRST(&vmem_tx->alloced_pages);
        SLIST_REMOVE_HEAD(&vmem_tx->alloced_pages, list);
    }

    return page;
}

static void
free_page(struct tm_vmem_tx* vmem_tx, struct tm_page* page)
{
    SLIST_INSERT_HEAD(&vmem_tx->alloced_pages, page, list);
}

static struct tm_page*
acquire_page_by_block(struct tm_vmem_tx* vmem_tx, size_t block_index,
                      struct picotm_error* error)
{
    /* Return existing page, if there is one... */

    struct tm_page* page;
    struct tm_page* prev = NULL;

    SLIST_FOREACH(page, &vmem_tx->active_pages, list) {
        size_t page_block_index = tm_page_block_index(page);
        if (page_block_index == block_index) {
            return page;
        } else if (page_block_index > block_index) {
            break;
        }
        prev = page;
    }

    /* ...or create a new page that refers to the corresponing frame. */

    page = alloc_page(vmem_tx, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    tm_page_init(page, block_index);

    /* The list of active pages is sorted by block index, so we
     * insert after the last page with a smaller block index, or
     * at the list's beginning. */

    if (prev) {
        SLIST_INSERT_AFTER(prev, page, list);
    } else {
        SLIST_INSERT_HEAD(&vmem_tx->active_pages, page, list);
    }

    return page;
}

static struct tm_page*
acquire_page_by_address(struct tm_vmem_tx* vmem_tx, uintptr_t addr,
                        struct picotm_error* error)
{
    return acquire_page_by_block(vmem_tx, tm_block_index_at(addr), error);
}

static void
release_page(struct tm_vmem_tx* vmem_tx, struct tm_page* page,
             struct picotm_error* error)
{
    if (tm_page_has_locked_frame(page)) {
        tm_page_unlock_frame(page, vmem_tx->vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
    SLIST_REMOVE(&vmem_tx->active_pages, page, tm_page, list);
    tm_page_uninit(page);
    free_page(vmem_tx, page);
}

static void
prepare_page_ld(struct tm_page* page, struct tm_vmem* vmem,
                struct picotm_error* error)
{
    if (tm_page_has_rdlocked_frame(page)) {
        return;
    }

    tm_page_try_rdlock_frame(page, vmem, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    tm_page_ld(page, vmem, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
tm_vmem_tx_ld(struct tm_vmem_tx* vmem_tx, uintptr_t addr, void* buf,
              size_t siz, struct picotm_error* error)
{
    uint8_t* buf8 = buf;

    while (siz) {

        /* released as part of apply() or undo() */
        struct tm_page* page = acquire_page_by_address(vmem_tx, addr, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        prepare_page_ld(page, vmem_tx->vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        size_t page_tail = TM_BLOCK_SIZE - page_head;
        size_t page_diff = siz < page_tail ? siz : page_tail;

        const uint8_t* mem = tm_page_buffer(page);
        memcpy(buf8, mem + page_head, page_diff);

        addr += page_diff;
        buf8 += page_diff;
        siz  -= page_diff;
    }
}

static void
prepare_page_st(struct tm_page* page, struct tm_vmem* vmem,
                struct picotm_error* error)
{
    if (tm_page_has_wrlocked_frame(page)) {
        return;
    }

    tm_page_try_wrlock_frame(page, vmem, error);
    if (picotm_error_is_set(error)) {
        return;
    }
    tm_page_ld(page, vmem, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
tm_vmem_tx_st(struct tm_vmem_tx* vmem_tx, uintptr_t addr, const void* buf,
              size_t siz, struct picotm_error* error)
{
    const uint8_t* buf8 = buf;

    while (siz) {

        /* released as part of apply() or undo() */
        struct tm_page* page = acquire_page_by_address(vmem_tx, addr, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        prepare_page_st(page, vmem_tx->vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        size_t page_tail = TM_BLOCK_SIZE - page_head;
        size_t page_diff = siz < page_tail ? siz : page_tail;

        uint8_t* mem = tm_page_buffer(page);
        memcpy(mem + page_head, buf8, page_diff);

        addr += page_diff;
        buf8 += page_diff;
        siz  -= page_diff;
    }
}

void
tm_vmem_tx_ldst(struct tm_vmem_tx* vmem_tx, uintptr_t laddr, uintptr_t saddr,
                size_t siz, struct picotm_error* error)
{
    while (siz) {

        /* released as part of apply() or undo() */
        struct tm_page* lpage = acquire_page_by_address(vmem_tx, laddr, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        prepare_page_ld(lpage, vmem_tx->vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        uintptr_t lpage_addr = tm_page_address(lpage);
        size_t lpage_head = laddr - lpage_addr;
        size_t lpage_tail = TM_BLOCK_SIZE - lpage_head;
        size_t lpage_diff = siz < lpage_tail ? siz : lpage_tail;

        /* released as part of apply() or undo() */
        struct tm_page* spage = acquire_page_by_address(vmem_tx, saddr, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        prepare_page_st(spage, vmem_tx->vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        uintptr_t spage_addr = tm_page_address(spage);
        size_t spage_head = saddr - spage_addr;
        size_t spage_tail = TM_BLOCK_SIZE - spage_head;
        size_t spage_diff = siz < spage_tail ? siz : spage_tail;

        uint8_t* lmem = tm_page_buffer(lpage);
        uint8_t* smem = tm_page_buffer(spage);
        memcpy(lmem + lpage_head, smem + spage_head, lpage_diff);

        laddr += lpage_diff;
        saddr += spage_diff;
        siz   -= lpage_diff;
    }
}

static void
prepare_page_privatize(struct tm_page* page, struct tm_vmem* vmem,
                       unsigned long flags, struct picotm_error* error)
{
    if ((flags & PICOTM_TM_PRIVATIZE_STORE) &&
            !tm_page_has_wrlocked_frame(page)) {

        /* Page requires a writer lock. */

        tm_page_try_wrlock_frame(page, vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        tm_page_ld(page, vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;

    } else if (!tm_page_has_rdlocked_frame(page)) {

        /* Page requires a reader lock. */

        tm_page_try_rdlock_frame(page, vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        tm_page_ld(page, vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;

    } else if (!(page->flags & TM_PAGE_FLAG_WRITE_THROUGH)) {

        /* Page holds correct lock, but requires write-through
         * mode. Privatized pages use write-through semantics,
         * so that all stores are immediately visible in the
         * region's memory.
         */

        if (tm_page_has_wrlocked_frame(page)) {
            tm_page_xchg(page, vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;
    }
}

void
tm_vmem_tx_privatize(struct tm_vmem_tx* vmem_tx, uintptr_t addr, size_t siz,
                     unsigned long flags, struct picotm_error* error)
{
    while (siz) {

        /* released as part of apply() or undo() */
        struct tm_page* page = acquire_page_by_address(vmem_tx, addr, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        prepare_page_privatize(page, vmem_tx->vmem, flags, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        size_t page_tail = TM_BLOCK_SIZE - page_head;
        size_t page_diff = siz < page_tail ? siz : page_tail;

        addr += page_diff;
        siz  -= page_diff;
    }
}

void
tm_vmem_tx_privatize_c(struct tm_vmem_tx* vmem_tx, uintptr_t addr, int c,
                       unsigned long flags, struct picotm_error* error)
{
    while (true) {

        /* released as part of apply() or undo() */
        struct tm_page* page = acquire_page_by_address(vmem_tx, addr, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        prepare_page_privatize(page, vmem_tx->vmem, flags, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        size_t page_tail = TM_BLOCK_SIZE - page_head;
        size_t page_diff = page_tail;

        /* check for 'c' in current page */
        const uint8_t* mem = tm_page_buffer(page);
        if (memchr(mem + page_head, c, page_diff)) {
            break;
        }

        addr += page_diff;
    }
}

void
tm_vmem_tx_lock(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    /* We've already locked all frames. */
}

void
tm_vmem_tx_unlock(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    /* The lock() function is a NO-OP, so this is as well. */
}

void
tm_vmem_tx_validate(struct tm_vmem_tx* vmem_tx, bool eotx,
                    struct picotm_error* error)
{
    /* We've locked all frames during execution phase, so we
     * already know that they are in a consistent state. */
}

void
tm_vmem_tx_apply(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    struct tm_page* page;
    SLIST_FOREACH(page, &vmem_tx->active_pages, list) {
        if (tm_page_has_wrlocked_frame(page) &&
                !(page->flags & TM_PAGE_FLAG_WRITE_THROUGH)) {
            tm_page_st(page, vmem_tx->vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
    }
}

void
tm_vmem_tx_undo(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    struct tm_page* page;
    SLIST_FOREACH(page, &vmem_tx->active_pages, list) {
        if (tm_page_has_wrlocked_frame(page) &&
                (page->flags & TM_PAGE_FLAG_WRITE_THROUGH)) {
            tm_page_st(page, vmem_tx->vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
    }
}

void
tm_vmem_tx_finish(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    while (!SLIST_EMPTY(&vmem_tx->active_pages)) {
        /* release_page() will remove page from queue */
        release_page(vmem_tx, SLIST_FIRST(&vmem_tx->active_pages), error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}
