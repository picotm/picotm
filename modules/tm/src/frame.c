/* Permission is hereby granted, free of charge, to any person obtaining a
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

#include "frame.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <stdatomic.h>
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

void
tm_frame_try_lock(struct tm_frame* frame, const void* owner,
                  struct picotm_error* error)
{
    /* test-and-test-and-set */
    if (frame->owner) {
        picotm_error_set_conflicting(error, NULL);
        return;
    }

    uintptr_t expected = 0;
    bool succ = atomic_compare_exchange_strong_explicit(&frame->owner,
                                                        &expected,
                                                        (uintptr_t)owner,
                                                        memory_order_seq_cst,
                                                        memory_order_acquire);
    if (!succ) {
        picotm_error_set_conflicting(error, NULL);
        return;
    }
}

void
tm_frame_unlock(struct tm_frame* frame)
{
    /* test-and-test-and-set */
    if (frame->owner) {
        atomic_store_explicit(&frame->owner, 0, memory_order_seq_cst);
    }
}
