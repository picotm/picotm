/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
