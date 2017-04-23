/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "frame.h"
#include <errno.h>
#include <stdbool.h>
#include "block.h"

void
tm_frame_init(struct tm_frame* frame, size_t block_index)
{
    frame->owner = 0;
    frame->flags = block_index << TM_BLOCK_SIZE_BITS;
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
    return frame->flags & ~TM_BLOCK_MASK;
}

int
tm_frame_try_lock(struct tm_frame* frame, const void* owner)
{
    /* test-and-test-and-set */
    if (frame->owner) {
        return -EBUSY;
    }

    uintptr_t expected = 0;
    bool succ = __atomic_compare_exchange_n(&frame->owner, &expected,
                                            (uintptr_t)owner, false,
                                            __ATOMIC_SEQ_CST,
                                            __ATOMIC_ACQUIRE);
    if (!succ) {
        return -EBUSY;
    }
    return 0;
}

void
tm_frame_unlock(struct tm_frame* frame)
{
    /* test-and-test-and-set */
    if (frame->owner) {
        __atomic_store_n(&frame->owner, 0, __ATOMIC_SEQ_CST);
    }
}
