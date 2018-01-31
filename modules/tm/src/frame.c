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

#include "frame.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-rwstate.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "block.h"

#define TM_FRAME_FLAGS_MASK     (TM_BLOCK_SIZE - 1)

void
tm_frame_init(struct tm_frame* frame, size_t block_index)
{
    picotm_rwlock_init(&frame->rwlock);
    frame->flags = block_index << TM_BLOCK_SIZE_BITS;
}

void
tm_frame_uninit(struct tm_frame* frame)
{
    picotm_rwlock_uninit(&frame->rwlock);
}

size_t
tm_frame_block_index(const struct tm_frame* frame)
{
    return frame->flags >> TM_BLOCK_SIZE_BITS;
}

uintptr_t
tm_frame_address(const struct tm_frame* frame)
{
    return tm_frame_block_index(frame) * TM_BLOCK_SIZE;
}

void*
tm_frame_buffer(const struct tm_frame* frame)
{
    return (void*)tm_frame_address(frame);
}

unsigned long
tm_frame_flags(const struct tm_frame* frame)
{
    return frame->flags & TM_FRAME_FLAGS_MASK;
}

void
tm_frame_try_rdlock(struct tm_frame* frame, struct picotm_rwstate* rwstate,
                    struct picotm_error* error)
{
    picotm_rwstate_try_rdlock(rwstate, &frame->rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
tm_frame_try_wrlock(struct tm_frame* frame, struct picotm_rwstate* rwstate,
                    struct picotm_error* error)
{
    picotm_rwstate_try_wrlock(rwstate, &frame->rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
tm_frame_unlock(struct tm_frame* frame, struct picotm_rwstate* rwstate)
{
    picotm_rwstate_unlock(rwstate, &frame->rwlock);
}
