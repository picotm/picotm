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
 */

#include "page.h"
#include <picotm/picotm-error.h>
#include <string.h>
#include "frame.h"
#include "vmem.h"

void
tm_page_init(struct tm_page* page, size_t block_index)
{
    page->flags = block_index << TM_BLOCK_SIZE_BITS;
    picotm_rwstate_init(&page->rwstate);
    memset(&page->list, 0, sizeof(page->list));
}

void
tm_page_uninit(struct tm_page* page)
{
    picotm_rwstate_uninit(&page->rwstate);
}

void*
tm_page_buffer(struct tm_page* page)
{
    if (page->flags & TM_PAGE_FLAG_WRITE_THROUGH) {
        return (void*)tm_page_address(page);
    }
    return page->buf; /* write-back is default mode */
}

void
tm_page_ld(struct tm_page* page, struct tm_vmem* vmem,
           struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        /* There's no legal way we should end up here! */
        picotm_error_mark_as_non_recoverable(error);
        return;
    }

    void* mem = tm_frame_buffer(frame);
    memcpy(page->buf, mem, sizeof(page->buf));

    tm_vmem_release_frame(vmem, frame);
}

void
tm_page_st(struct tm_page* page, struct tm_vmem* vmem,
           struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        /* There's no legal way we should end up here! */
        picotm_error_mark_as_non_recoverable(error);
        return;
    }

    void* buf = tm_frame_buffer(frame);
    memcpy(buf, page->buf, sizeof(page->buf));

    tm_vmem_release_frame(vmem, frame);
}

void
tm_page_xchg(struct tm_page* page, struct tm_vmem* vmem,
             struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        /* There's no legal way we should end up here! */
        picotm_error_mark_as_non_recoverable(error);
        return;
    }

    uint8_t buf[TM_BLOCK_SIZE];
    void* fbuf = tm_frame_buffer(frame);
    void* pbuf = page->buf;

    memcpy( buf, fbuf, sizeof(buf));
    memcpy(fbuf, pbuf, sizeof(buf));
    memcpy(pbuf,  buf, sizeof(buf));

    tm_vmem_release_frame(vmem, frame);
}

void
tm_page_try_rdlock_frame(struct tm_page* page, struct tm_vmem* vmem,
                         struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        return;
    }
    tm_frame_try_rdlock(frame, &page->rwstate, error);
    if (picotm_error_is_set(error)) {
        goto err_tm_frame_lock;
    }
    tm_vmem_release_frame(vmem, frame);

    return;

err_tm_frame_lock:
    tm_vmem_release_frame(vmem, frame);
}

void
tm_page_try_wrlock_frame(struct tm_page* page, struct tm_vmem* vmem,
                         struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        return;
    }
    tm_frame_try_wrlock(frame, &page->rwstate, error);
    if (picotm_error_is_set(error)) {
        goto err_tm_frame_lock;
    }
    tm_vmem_release_frame(vmem, frame);

    return;

err_tm_frame_lock:
    tm_vmem_release_frame(vmem, frame);
}

void
tm_page_unlock_frame(struct tm_page* page, struct tm_vmem* vmem,
                     struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        /* There's no legal way we should end up here! */
        picotm_error_mark_as_non_recoverable(error);
        return;
    }

    tm_frame_unlock(frame, &page->rwstate);
    tm_vmem_release_frame(vmem, frame);
}
