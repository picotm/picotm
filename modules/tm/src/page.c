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

#include "page.h"
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <string.h>
#include "frame.h"
#include "vmem.h"

void
tm_page_init(struct tm_page* page, size_t block_index)
{
    page->flags = block_index << TM_BLOCK_SIZE_BITS;
    picotm_rwstate_init(&page->rwstate);
    page->buf_bits = 0;
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

static bool
all_buf_bits_set(unsigned long bits)
{
    return bits == (TM_BLOCK_SIZE - 1);
}

void
tm_page_ld(struct tm_page* page, unsigned long bits, struct tm_vmem* vmem,
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

    if (all_buf_bits_set(bits) && !page->buf_bits) {
        void* mem = tm_frame_buffer(frame);
        memcpy(page->buf, mem, sizeof(page->buf));

    } else {
              uint8_t* buf_beg = picotm_arraybeg(page->buf);
        const uint8_t* buf_end = picotm_arrayend(page->buf);
        const uint8_t* mem = tm_frame_buffer(frame);
        unsigned long bit = 1ul;

        for (uint8_t* buf = buf_beg; buf < buf_end; ++buf, ++mem, bit <<= 1) {
            if ((bits & bit) && !(page->buf_bits & bit)) {
                *buf = *mem;
            }
        }
    }

    page->buf_bits |= bits;

    tm_vmem_release_frame(vmem, frame);
}

bool
tm_page_ld_c(struct tm_page* page, unsigned long bits, int c, struct tm_vmem* vmem,
             struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        /* There's no legal way we should end up here! */
        picotm_error_mark_as_non_recoverable(error);
        return false;
    }

          uint8_t* buf_beg = picotm_arraybeg(page->buf);
    const uint8_t* buf_end = picotm_arrayend(page->buf);
          uint8_t* buf;
    const uint8_t* mem = tm_frame_buffer(frame);
    unsigned long bit = 1ul;

    for (buf = buf_beg; (buf < buf_end) && (*mem != c); ++buf, ++mem, bit <<= 1) {
        if ((bits & bit) && !(page->buf_bits & bit)) {
            *buf = *mem;
        }
    }

    page->buf_bits |= bits;

    tm_vmem_release_frame(vmem, frame);

    return buf != buf_end;
}

void
tm_page_st(struct tm_page* page, unsigned long bits, struct tm_vmem* vmem,
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

    if (all_buf_bits_set(bits) && all_buf_bits_set(page->buf_bits)) {
        void* mem = tm_frame_buffer(frame);
        memcpy(mem, page->buf, sizeof(page->buf));

    } else {
        const uint8_t* buf_beg = picotm_arraybeg(page->buf);
        const uint8_t* buf_end = picotm_arrayend(page->buf);
              uint8_t* mem = tm_frame_buffer(frame);
        unsigned long bit = 1ul;

        for (const uint8_t* buf = buf_beg;
                            buf < buf_end;
                          ++buf, ++mem, bit <<= 1) {
            if ((bits & bit) && (page->buf_bits & bit)) {
                *mem = *buf;
            }
        }
    }

    tm_vmem_release_frame(vmem, frame);
}

void
tm_page_xchg(struct tm_page* page, unsigned long bits, struct tm_vmem* vmem,
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

    if (all_buf_bits_set(bits) && all_buf_bits_set(page->buf_bits)) {
        uint8_t buf[TM_BLOCK_SIZE];
        void* fbuf = tm_frame_buffer(frame);
        void* pbuf = page->buf;

        memcpy( buf, fbuf, sizeof(buf));
        memcpy(fbuf, pbuf, sizeof(buf));
        memcpy(pbuf,  buf, sizeof(buf));

    } else {
              uint8_t* buf_beg = picotm_arraybeg(page->buf);
        const uint8_t* buf_end = picotm_arrayend(page->buf);
              uint8_t* mem = tm_frame_buffer(frame);
        unsigned long bit = 1ul;

        for (uint8_t* buf = buf_beg; buf < buf_end; ++buf, ++mem, bit <<= 1) {
            if ((bits & bit) && (page->buf_bits & bit)) {
                *mem ^= *buf;
                *buf ^= *mem;
                *mem ^= *buf;
            }
        }
    }

    tm_vmem_release_frame(vmem, frame);
}

bool
tm_page_xchg_c(struct tm_page* page, unsigned long bits, int c, struct tm_vmem* vmem,
               struct picotm_error* error)
{
    struct tm_frame* frame =
        tm_vmem_acquire_frame_by_block(vmem, tm_page_block_index(page),
                                       error);
    if (picotm_error_is_set(error)) {
        /* There's no legal way we should end up here! */
        picotm_error_mark_as_non_recoverable(error);
        return false;
    }

          uint8_t* buf_beg = picotm_arraybeg(page->buf);
    const uint8_t* buf_end = picotm_arrayend(page->buf);
          uint8_t* buf;
          uint8_t* mem = tm_frame_buffer(frame);
    unsigned long bit = 1ul;

    for (buf = buf_beg; (buf < buf_end) && (*mem != c); ++buf, ++mem, bit <<= 1) {
        if ((bits & bit) && (page->buf_bits & bit)) {
            *mem ^= *buf;
            *buf ^= *mem;
            *mem ^= *buf;
        }
    }

    tm_vmem_release_frame(vmem, frame);

    return buf != buf_end;
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
