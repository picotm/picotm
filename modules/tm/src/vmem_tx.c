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

#include "vmem_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-tm.h"
#include <stdlib.h>
#include <string.h>
#include "page.h"

void
tm_vmem_tx_init(struct tm_vmem_tx* vmem_tx, struct tm_vmem* vmem,
                unsigned long module)
{
    vmem_tx->vmem = vmem;
    vmem_tx->module = module;

    picotm_slist_init_head(&vmem_tx->active_pages);
    picotm_slist_init_head(&vmem_tx->alloced_pages);
}

static void
cleanup_page(struct picotm_slist* item)
{
    struct tm_page* page = tm_page_of_slist(item);
    picotm_slist_uninit_item(&page->list);
    free(page);
}

void
tm_vmem_tx_uninit(struct tm_vmem_tx* vmem_tx)
{
    picotm_slist_cleanup_0(&vmem_tx->active_pages, cleanup_page);
    picotm_slist_uninit_head(&vmem_tx->active_pages);

    picotm_slist_cleanup_0(&vmem_tx->alloced_pages, cleanup_page);
    picotm_slist_uninit_head(&vmem_tx->alloced_pages);
}

static struct tm_page*
alloc_page(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    struct tm_page* page;

    if (picotm_slist_is_empty(&vmem_tx->alloced_pages)) {
        page = malloc(sizeof(*page));
        if (!page) {
            picotm_error_set_error_code(error, PICOTM_OUT_OF_MEMORY);
            return NULL;
        }
    } else {
        page = tm_page_of_slist(picotm_slist_front(&vmem_tx->alloced_pages));
        picotm_slist_dequeue_front(&vmem_tx->alloced_pages);
    }

    return page;
}

static void
free_page(struct tm_vmem_tx* vmem_tx, struct tm_page* page)
{
    picotm_slist_enqueue_front(&vmem_tx->alloced_pages, &page->list);
}

static _Bool
find_page_by_block_index(const struct tm_page* page, size_t block_index,
                         struct tm_page** prev)
{
    if (tm_page_block_index(page) >= block_index) {
        return true; /* stop iterating */
    }
    *prev = (struct tm_page*)page;
    return false;
}

static _Bool
find_page_by_block_index_cb(const struct picotm_slist* item,
                            void* data1, void* data2)
{
    const struct tm_page* page = tm_page_of_const_slist(item);
    size_t* block_index = data1;
    struct tm_page** prev = data2;
    return find_page_by_block_index(page, *block_index, prev);
}

static struct tm_page*
acquire_page_by_block(struct tm_vmem_tx* vmem_tx, size_t block_index,
                      struct picotm_error* error)
{
    /* Return existing page, if there is one... */

    struct tm_page* prev = NULL;

    struct picotm_slist* pos = picotm_slist_find_2(&vmem_tx->active_pages,
                                                   find_page_by_block_index_cb,
                                                   &block_index, &prev);
    if (pos != picotm_slist_end(&vmem_tx->active_pages)) {
        struct tm_page* page = tm_page_of_slist(pos);
        if (tm_page_block_index(page) == block_index) {
            return page;
        }
    }

    /* ...or create a new page that refers to the corresponding frame. */

    struct tm_page* page = alloc_page(vmem_tx, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    tm_page_init(page, block_index);

    /* The list of active pages is sorted by block index, so we
     * insert after the last page with a smaller block index, or
     * at the list's beginning. */

    if (prev) {
        picotm_slist_enqueue_after(&prev->list, &page->list);
    } else {
        picotm_slist_enqueue_front(&vmem_tx->active_pages, &page->list);
    }

    return page;
}

static struct tm_page*
acquire_page_by_address(struct tm_vmem_tx* vmem_tx, uintptr_t addr,
                        struct picotm_error* error)
{
    return acquire_page_by_block(vmem_tx, tm_block_index_at(addr), error);
}

static unsigned long
ulmin(unsigned long lhs, unsigned long rhs)
{
    return lhs < rhs ? lhs : rhs;
}

static unsigned long
copy_all_bits(void)
{
    return TM_BLOCK_OFFSET_MASK;
}

static unsigned long
copy_bits(uintptr_t addr, size_t siz)
{
    unsigned long bits = copy_all_bits(); /* Set all bits. */

    /* Don't copy more than siz or block-size bytes. */
    bits >>= TM_BLOCK_SIZE - ulmin(siz, TM_BLOCK_SIZE);
    /* Start copying at byte. */
    bits <<= addr - picotm_address_floor(addr, TM_BLOCK_SIZE);

    /* Filter out bits within the current page. */
    return bits & copy_all_bits();
}

static void
prepare_page_ld(struct tm_page* page, uintptr_t addr, size_t siz,
                struct tm_vmem* vmem,
                struct picotm_error* error)
{
    if (page->flags & TM_PAGE_FLAG_DISCARDED) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_BOUNDS);
        return;
    } else if (!tm_page_has_rdlocked_frame(page)) {
        tm_page_try_rdlock_frame(page, vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    if (tm_page_is_complete(page)) {
        return;
    }
    tm_page_ld(page, copy_bits(addr, siz), vmem, error);
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
        prepare_page_ld(page, addr, siz, vmem_tx->vmem, error);
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
prepare_page_st(struct tm_page* page, uintptr_t addr, size_t siz,
                struct tm_vmem* vmem,
                struct picotm_error* error)
{
    if (page->flags & TM_PAGE_FLAG_DISCARDED) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_BOUNDS);
        return;
    } else if (!tm_page_has_wrlocked_frame(page)) {
        tm_page_try_wrlock_frame(page, vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    if (tm_page_is_complete(page)) {
        return;
    }
    /* LD marks the bits we're going to store. */
    tm_page_ld(page, copy_bits(addr, siz), vmem, error);
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
        prepare_page_st(page, addr, siz, vmem_tx->vmem, error);
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
        prepare_page_ld(lpage, laddr, siz, vmem_tx->vmem, error);
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
        prepare_page_st(spage, saddr, siz, vmem_tx->vmem, error);
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
prepare_page_privatize(struct tm_page* page, uintptr_t addr, size_t siz,
                       struct tm_vmem* vmem,
                       unsigned long flags, struct picotm_error* error)
{
    if (page->flags & TM_PAGE_FLAG_DISCARDED) {
        picotm_error_set_error_code(error, PICOTM_OUT_OF_BOUNDS);
        return;
    }

    if (flags & PICOTM_TM_PRIVATIZE_STORE) {

        /* Page requires a writer lock. */

        if (!tm_page_has_wrlocked_frame(page)) {
            tm_page_try_wrlock_frame(page, vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        if (!tm_page_is_complete(page)) {
            tm_page_ld(page, copy_bits(addr, siz), vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;

    } else if (flags & PICOTM_TM_PRIVATIZE_LOAD) {

        /* Page requires a reader lock. */

        if (!tm_page_has_rdlocked_frame(page)) {
            tm_page_try_rdlock_frame(page, vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        if (!tm_page_is_complete(page)) {
            tm_page_ld(page, copy_bits(addr, siz), vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;

    } else if (!flags) {

        /* Not setting any flags marks the page as discarded.
         * Page requires a writer lock. */

        if (!tm_page_has_wrlocked_frame(page)) {
            tm_page_try_wrlock_frame(page, vmem, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }
        page->flags |= TM_PAGE_FLAG_DISCARDED;

    } else if (!(page->flags & TM_PAGE_FLAG_WRITE_THROUGH)) {

        /* Page holds correct lock, but requires write-through
         * mode. Privatized pages use write-through semantics,
         * so that all stores are immediately visible in the
         * region's memory.
         */

        if (tm_page_has_wrlocked_frame(page)) {
            tm_page_xchg(page, copy_bits(addr, siz), vmem, error);
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
        prepare_page_privatize(page, addr, siz, vmem_tx->vmem, flags, error);
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

static bool
prepare_page_privatize_c(struct tm_page* page,
                         uintptr_t addr, size_t page_diff, int c,
                         struct tm_vmem* vmem, unsigned long flags,
                         struct picotm_error* error)
{
    bool found_c;

    if (flags & PICOTM_TM_PRIVATIZE_STORE) {

        /* Page requires a writer lock. */

        if (!tm_page_has_wrlocked_frame(page)) {
            tm_page_try_wrlock_frame(page, vmem, error);
            if (picotm_error_is_set(error)) {
                return false;
            }
        }

        found_c = tm_page_ld_c(page, copy_bits(addr, page_diff), c, vmem,
                               error);
        if (picotm_error_is_set(error)) {
            return false;
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;

    } else if (flags & PICOTM_TM_PRIVATIZE_LOAD) {

        /* Page requires a reader lock. */

        if (!tm_page_has_rdlocked_frame(page)) {
            tm_page_try_rdlock_frame(page, vmem, error);
            if (picotm_error_is_set(error)) {
                return false;
            }
        }
        found_c = tm_page_ld_c(page, copy_bits(addr, page_diff), c, vmem,
                               error);
        if (picotm_error_is_set(error)) {
            return false;
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;

    } else if (!flags) {

        /* Not setting any flags marks the page as discarded.
         * Page requires a writer lock. */

        if (!tm_page_has_wrlocked_frame(page)) {
            tm_page_try_wrlock_frame(page, vmem, error);
            if (picotm_error_is_set(error)) {
                return false;
            }
        }

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        const uint8_t* mem = tm_page_buffer(page);
        found_c = !!memchr(mem + page_head, c, page_diff);

        page->flags |= TM_PAGE_FLAG_DISCARDED;

    } else if (!(page->flags & TM_PAGE_FLAG_WRITE_THROUGH)) {

        /* Page holds correct lock, but requires write-through
         * mode. Privatized pages use write-through semantics,
         * so that all stores are immediately visible in the
         * region's memory.
         */

        if (tm_page_has_wrlocked_frame(page)) {
            found_c = tm_page_xchg_c(page, copy_bits(addr, page_diff), c, vmem, error);
            if (picotm_error_is_set(error)) {
                return false;
            }
        } else {
            uintptr_t page_addr = tm_page_address(page);
            size_t page_head = addr - page_addr;
            const uint8_t* mem = tm_page_buffer(page);
            found_c = !!memchr(mem + page_head, c, page_diff);
        }
        page->flags |= TM_PAGE_FLAG_WRITE_THROUGH;
    } else {

        /* The page is already set up correctly. We only have to
         * check for the character 'c'.
         */

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        const uint8_t* mem = tm_page_buffer(page);
        found_c = !!memchr(mem + page_head, c, page_diff);
    }

    return found_c;
}

void
tm_vmem_tx_privatize_c(struct tm_vmem_tx* vmem_tx, uintptr_t addr, int c,
                       unsigned long flags, struct picotm_error* error)
{
    bool found_c = false;

    while (!found_c) {

        /* released as part of apply() or undo() */
        struct tm_page* page = acquire_page_by_address(vmem_tx, addr, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        size_t page_tail = TM_BLOCK_SIZE - page_head;
        size_t page_diff = page_tail;

        found_c = prepare_page_privatize_c(page, addr, page_diff, c,
                                           vmem_tx->vmem, flags, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        addr += page_diff;
    }
}

static size_t
apply_page(struct tm_page* page, struct tm_vmem* vmem, struct picotm_error* error)
{
    if (tm_page_has_wrlocked_frame(page) &&
        !(page->flags & TM_PAGE_FLAG_WRITE_THROUGH) &&
        !(page->flags & TM_PAGE_FLAG_DISCARDED)) {
        tm_page_st(page, copy_all_bits(), vmem, error);
        if (picotm_error_is_set(error)) {
            return 0;
        }
    }
    return 1;
}

static size_t
apply_page_cb(struct picotm_slist* item, void* data1, void* data2)
{
    return apply_page(tm_page_of_slist(item), data1, data2);
}

void
tm_vmem_tx_apply(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    picotm_slist_walk_2(&vmem_tx->active_pages, apply_page_cb,
                        vmem_tx->vmem, error);
}

static size_t
undo_page(struct tm_page* page, struct tm_vmem* vmem,
          struct picotm_error* error)
{
    if (tm_page_has_wrlocked_frame(page) &&
        (page->flags & TM_PAGE_FLAG_WRITE_THROUGH) &&
        !(page->flags & TM_PAGE_FLAG_DISCARDED)) {
        tm_page_st(page, copy_all_bits(), vmem, error);
        if (picotm_error_is_set(error)) {
            return 0;
        }
    }
    return 1;
}

static size_t
undo_page_cb(struct picotm_slist* item, void* data1, void* data2)
{
    return undo_page(tm_page_of_slist(item), data1, data2);
}

void
tm_vmem_tx_undo(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    picotm_slist_walk_2(&vmem_tx->active_pages, undo_page_cb,
                        vmem_tx->vmem, error);
}

static void
finish_page(struct tm_page* page, struct tm_vmem_tx* vmem_tx,
            struct picotm_error* error)
{
    if (tm_page_has_locked_frame(page)) {
        tm_page_unlock_frame(page, vmem_tx->vmem, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    tm_page_uninit(page);
    free_page(vmem_tx, page);
}

static void
finish_page_cb(struct picotm_slist* item, void* data1, void* data2)
{
    finish_page(tm_page_of_slist(item), data1, data2);
}

void
tm_vmem_tx_finish(struct tm_vmem_tx* vmem_tx, struct picotm_error* error)
{
    picotm_slist_cleanup_2(&vmem_tx->active_pages, finish_page_cb, vmem_tx,
                           error);
}
