/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vmem_tx.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "block.h"
#include "vmem.h"

/*
 * |struct tm_page|
 */

enum {
    TM_PAGE_FLAG_OWNING_FRAME  = 1 << 0,
    TM_PAGE_FLAG_WRITTEN       = 1 << 1,
    TM_PAGE_FLAG_WRITE_THROUGH = 1 << 2
};

/**
 * |struct tm_page| represents the transaction-local state of a block
 * of memory. The global counterpart is called |struct tm_frame|.
 */
struct tm_page {
    /** Block index and flag bits */
    uintptr_t flags;

    /** Transaction-local buffer */
    uint8_t buf[TM_BLOCK_SIZE];

    /** Entry into allocator lists */
    SLIST_ENTRY(tm_page) list;
};

static int
tm_page_init(struct tm_page* page, struct tm_vmem* vmem, size_t block_index)
{
    page->flags = block_index << TM_BLOCK_SIZE_BITS;
    memset(&page->list, 0, sizeof(page->list));

    return 0;
}

static size_t
tm_page_block_index(const struct tm_page* page)
{
    return page->flags >> TM_BLOCK_SIZE_BITS;
}

static uintptr_t
tm_page_address(const struct tm_page* page)
{
    return tm_page_block_index(page) * TM_BLOCK_SIZE;
}

static void*
tm_page_buffer(struct tm_page* page)
{
    if (page->flags & TM_PAGE_FLAG_WRITE_THROUGH) {
        return (void*)tm_page_address(page);
    }
    return page->buf; /* write-back is default mode */
}

static int
tm_page_ld(struct tm_page* page, struct tm_vmem* vmem)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page));
    if (!frame) {
        abort(); /* There's no legal way we should end up here! */
    }

    void* mem = tm_frame_buffer(frame);
    memcpy(page->buf, mem, sizeof(page->buf));

    tm_vmem_release_frame(vmem, frame);

    return 0;
}

static int
tm_page_st(struct tm_page* page, struct tm_vmem* vmem)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page));
    if (!frame) {
        abort(); /* There's no legal way we should end up here! */
    }

    void* buf = tm_frame_buffer(frame);
    memcpy(buf, page->buf, sizeof(page->buf));

    tm_vmem_release_frame(vmem, frame);

    return 0;
}

static int
tm_page_xchg(struct tm_page* page, struct tm_vmem* vmem)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page));
    if (!frame) {
        abort(); /* There's no legal way we should end up here! */
    }

    uint8_t buf[TM_BLOCK_SIZE];
    void* fbuf = tm_frame_buffer(frame);
    void* pbuf = page->buf;

    memcpy( buf, fbuf, sizeof(buf));
    memcpy(fbuf, pbuf, sizeof(buf));
    memcpy(pbuf,  buf, sizeof(buf));

    tm_vmem_release_frame(vmem, frame);

    return 0;
}

static int
tm_page_try_lock_frame(struct tm_page* page, struct tm_vmem* vmem)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page));
    if (!frame) {
        return -ENOMEM;
    }
    int res = tm_frame_try_lock(frame, page);
    if (res < 0) {
        goto err_tm_frame_lock;
    }
    tm_vmem_release_frame(vmem, frame);

    page->flags |= TM_PAGE_FLAG_OWNING_FRAME;

    return 0;

err_tm_frame_lock:
    tm_vmem_release_frame(vmem, frame);
    return res;
}

static void
tm_page_unlock_frame(struct tm_page* page, struct tm_vmem* vmem)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page));
    if (!frame) {
        abort(); /* There's no legal way we should end up here! */
    }
    tm_frame_unlock(frame);
    tm_vmem_release_frame(vmem, frame);

    page->flags &= ~TM_PAGE_FLAG_OWNING_FRAME;
}

/*
 * |struct tm_vmem_tx|
 */

int
tm_vmem_tx_init(struct tm_vmem_tx* vmem_tx, struct tm_vmem* vmem,
                unsigned long module)
{
    vmem_tx->vmem = vmem;
    vmem_tx->module = module;

    SLIST_INIT(&vmem_tx->active_pages);
    SLIST_INIT(&vmem_tx->alloced_pages);

    vmem_tx->is_initialized = true;

    return 0;
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
alloc_page(struct tm_vmem_tx* vmem_tx)
{
    struct tm_page* page;

    if (SLIST_EMPTY(&vmem_tx->alloced_pages)) {
        page = malloc(sizeof(*page));
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
acquire_page_by_block(struct tm_vmem_tx* vmem_tx, size_t block_index)
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

    page = alloc_page(vmem_tx);
    if (!page) {
        return NULL;
    }

    int res = tm_page_init(page, vmem_tx->vmem, block_index);
    if (res < 0) {
        goto err_tm_page_init;
    }

    /* The list of active pages is sorted by block index, so we
     * insert after the last page with a smaller block index, or
     * at the list's beginning. */

    if (prev) {
        SLIST_INSERT_AFTER(prev, page, list);
    } else {
        SLIST_INSERT_HEAD(&vmem_tx->active_pages, page, list);
    }

    return page;

err_tm_page_init:
    free_page(vmem_tx, page);
    return NULL;
}

static struct tm_page*
acquire_page_by_address(struct tm_vmem_tx* vmem_tx, uintptr_t addr)
{
    return acquire_page_by_block(vmem_tx, tm_block_index_at(addr));
}

static void
release_page(struct tm_vmem_tx* vmem_tx, struct tm_page* page)
{
    if (page->flags & TM_PAGE_FLAG_OWNING_FRAME) {
        tm_page_unlock_frame(page, vmem_tx->vmem);
    }
    SLIST_REMOVE(&vmem_tx->active_pages, page, tm_page, list);
    free_page(vmem_tx, page);
}

int
tm_vmem_tx_ld(struct tm_vmem_tx* vmem_tx, uintptr_t addr, void* buf,
              size_t siz)
{
    uint8_t* buf8 = buf;

    while (siz) {

        /* released as part of apply() or undo() */
        struct tm_page* page = acquire_page_by_address(vmem_tx, addr);
        if (!page) {
            return -ENOMEM;
        }

        if (!(page->flags & TM_PAGE_FLAG_OWNING_FRAME)) {
            int res = tm_page_try_lock_frame(page, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
            res = tm_page_ld(page, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
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

    return 0;
}

int
tm_vmem_tx_st(struct tm_vmem_tx* vmem_tx, uintptr_t addr, const void* buf,
              size_t siz)
{
    const uint8_t* buf8 = buf;

    while (siz) {

        /* released as part of apply() or undo() */
        struct tm_page* page = acquire_page_by_address(vmem_tx, addr);
        if (!page) {
            return -ENOMEM;
        }

        if (!(page->flags & TM_PAGE_FLAG_OWNING_FRAME)) {
            int res = tm_page_try_lock_frame(page, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
            res = tm_page_ld(page, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
        }

        uintptr_t page_addr = tm_page_address(page);
        size_t page_head = addr - page_addr;
        size_t page_tail = TM_BLOCK_SIZE - page_head;
        size_t page_diff = siz < page_tail ? siz : page_tail;

        uint8_t* mem = tm_page_buffer(page);
        memcpy(mem + page_head, buf8, page_diff);

        page->flags |= TM_PAGE_FLAG_WRITTEN;

        addr += page_diff;
        buf8 += page_diff;
        siz  -= page_diff;
    }

    return 0;
}

int
tm_vmem_tx_ldst(struct tm_vmem_tx* vmem_tx, uintptr_t laddr, uintptr_t saddr,
                size_t siz)
{
    while (siz) {

        /* released as part of apply() or undo() */
        struct tm_page* lpage = acquire_page_by_address(vmem_tx, laddr);
        if (!lpage) {
            return -ENOMEM;
        }

        if (!(lpage->flags & TM_PAGE_FLAG_OWNING_FRAME)) {
            int res = tm_page_try_lock_frame(lpage, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
            res = tm_page_ld(lpage, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
        }

        uintptr_t lpage_addr = tm_page_address(lpage);
        size_t lpage_head = laddr - lpage_addr;
        size_t lpage_tail = TM_BLOCK_SIZE - lpage_head;
        size_t lpage_diff = siz < lpage_tail ? siz : lpage_tail;

        /* released as part of apply() or undo() */
        struct tm_page* spage = acquire_page_by_address(vmem_tx, saddr);
        if (!spage) {
            return -ENOMEM;
        }

        if (!(spage->flags & TM_PAGE_FLAG_OWNING_FRAME)) {
            int res = tm_page_try_lock_frame(spage, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
            res = tm_page_ld(spage, vmem_tx->vmem);
            if (res < 0) {
                return res;
            }
        }

        uintptr_t spage_addr = tm_page_address(spage);
        size_t spage_head = saddr - spage_addr;
        size_t spage_tail = TM_BLOCK_SIZE - spage_head;
        size_t spage_diff = siz < spage_tail ? siz : spage_tail;

        uint8_t* lmem = tm_page_buffer(lpage);
        uint8_t* smem = tm_page_buffer(spage);
        memcpy(lmem + lpage_head, smem + spage_head, lpage_diff);

        spage->flags |= TM_PAGE_FLAG_WRITTEN;

        laddr += lpage_diff;
        saddr += spage_diff;
        siz   -= lpage_diff;
    }

    return 0;
}

int
tm_vmem_tx_lock(struct tm_vmem_tx* vmem_tx)
{
    /* We've already locked all frames. */
    return 0;
}

int
tm_vmem_tx_unlock(struct tm_vmem_tx* vmem_tx)
{
    /* The lock() function is a NO-OP, so this is as well. */
    return 0;
}

int
tm_vmem_tx_validate(struct tm_vmem_tx* vmem_tx, bool eotx)
{
    /* We've locked all frames during execution phase, so we
     * already know that they are in a consistent state. */
    return 0;
}

int
tm_vmem_tx_apply(struct tm_vmem_tx* vmem_tx)
{
    struct tm_page* page;
    SLIST_FOREACH(page, &vmem_tx->active_pages, list) {
        if ((page->flags & TM_PAGE_FLAG_WRITTEN) &&
            !(page->flags & TM_PAGE_FLAG_WRITE_THROUGH)) {
            tm_page_st(page, vmem_tx->vmem);
        }
    }
    return 0;
}

int
tm_vmem_tx_undo(struct tm_vmem_tx* vmem_tx)
{
    struct tm_page* page;
    SLIST_FOREACH(page, &vmem_tx->active_pages, list) {
        if ((page->flags & TM_PAGE_FLAG_WRITTEN) &&
            (page->flags & TM_PAGE_FLAG_WRITE_THROUGH)) {
            tm_page_st(page, vmem_tx->vmem);
        }
    }
    return 0;
}

int
tm_vmem_tx_finish(struct tm_vmem_tx* vmem_tx)
{
    while (!SLIST_EMPTY(&vmem_tx->active_pages)) {
        /* release_page() will remove page from queue */
        release_page(vmem_tx, SLIST_FIRST(&vmem_tx->active_pages));
    }
    return 0;
}
