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

#include "frame.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-rwstate.h"
#include <errno.h>
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
