/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vmem.h"
#include <picotm/picotm-module.h>
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
