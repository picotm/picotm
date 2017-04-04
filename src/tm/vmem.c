/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vmem.h"
#include <systx/systx-module.h>
#include "block.h"
#include "frame.h"

int
tm_vmem_init(struct tm_vmem* vmem)
{
    int res = tm_frame_map_init(&vmem->frame_map);
    if (res < 0) {
        return res;
    }
    return 0;
}

void
tm_vmem_uninit(struct tm_vmem* vmem)
{
    tm_frame_map_uninit(&vmem->frame_map);
}

struct tm_frame*
tm_vmem_acquire_frame_by_block(struct tm_vmem* vmem, size_t block_index)
{
    return tm_vmem_acquire_frame_by_address(vmem,
                                            block_index << TM_BLOCK_SIZE_BITS);
}

struct tm_frame*
tm_vmem_acquire_frame_by_address(struct tm_vmem* vmem, uintptr_t addr)
{
    return tm_frame_map_lookup(&vmem->frame_map, addr);
}

void
tm_vmem_release_frame(struct tm_vmem* vmem, struct tm_frame* frame)
{
}
