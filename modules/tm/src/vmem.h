/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "framemap.h"

struct picotm_error;

/**
 * \cond impl || tm_impl
 * \ingroup tm_impl
 * \file
 * \endcond
 */

/**
 * |struct tm_vmem| represents main memory; the resource that
 * the TM module maintains.
 */
struct tm_vmem {
    struct tm_frame_map frame_map;
};

void
tm_vmem_init(struct tm_vmem* vmem);

void
tm_vmem_uninit(struct tm_vmem* vmem);

struct tm_frame*
tm_vmem_acquire_frame_by_block(struct tm_vmem* vmem, size_t block_index,
                               struct picotm_error* error);

struct tm_frame*
tm_vmem_acquire_frame_by_address(struct tm_vmem* vmem, uintptr_t addr,
                                 struct picotm_error* error);

void
tm_vmem_release_frame(struct tm_vmem* vmem, struct tm_frame* frame);
