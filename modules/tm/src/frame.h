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
