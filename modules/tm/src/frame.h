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

#pragma once

#include "picotm/picotm-lib-rwlock.h"
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

struct picotm_error;
struct picotm_rwstate;

/**
 * \cond impl || tm_impl
 * \ingroup tm_impl
 * \file
 * \endcond
 */

/**
 * |struct tm_frame| represents a single contiguous block
 * of main memory.
 */
struct tm_frame {
    struct picotm_rwlock rwlock; /* R/W lock */
    uintptr_t flags; /* address + flags */
};

void
tm_frame_init(struct tm_frame* frame, size_t block_index);

void
tm_frame_uninit(struct tm_frame* frame);

size_t
tm_frame_block_index(const struct tm_frame* frame);

uintptr_t
tm_frame_address(const struct tm_frame* frame);

void*
tm_frame_buffer(const struct tm_frame* frame);

unsigned long
tm_frame_flags(const struct tm_frame* frame);

void
tm_frame_try_rdlock(struct tm_frame* frame, struct picotm_rwstate* rwstate,
                    struct picotm_error* error);

void
tm_frame_try_wrlock(struct tm_frame* frame, struct picotm_rwstate* rwstate,
                    struct picotm_error* error);

void
tm_frame_unlock(struct tm_frame* frame, struct picotm_rwstate* rwstate);
