/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
