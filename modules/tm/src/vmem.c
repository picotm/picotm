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

#include "vmem.h"
#include "picotm/picotm-module.h"
#include "block.h"
#include "frame.h"

void
tm_vmem_init(struct tm_vmem* vmem)
{
    tm_frame_map_init(&vmem->frame_map);
}

void
tm_vmem_uninit(struct tm_vmem* vmem)
{
    tm_frame_map_uninit(&vmem->frame_map);
}

struct tm_frame*
tm_vmem_acquire_frame_by_block(struct tm_vmem* vmem, size_t block_index,
                               struct picotm_error* error)
{
    return tm_vmem_acquire_frame_by_address(vmem,
                                            block_index << TM_BLOCK_SIZE_BITS,
                                            error);
}

struct tm_frame*
tm_vmem_acquire_frame_by_address(struct tm_vmem* vmem, uintptr_t addr,
                                 struct picotm_error* error)
{
    return tm_frame_map_lookup(&vmem->frame_map, addr, error);
}

void
tm_vmem_release_frame(struct tm_vmem* vmem, struct tm_frame* frame)
{
}
