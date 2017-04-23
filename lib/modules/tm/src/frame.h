/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * |struct tm_frame| represents a single contiguous block
 * of main memory.
 */
struct tm_frame {
    uintptr_t owner; /* address of owning vmem_tx, or 0 */
    uintptr_t flags; /* address + flags */
};

void
tm_frame_init(struct tm_frame* frame, size_t block_index);

size_t
tm_frame_block_index(const struct tm_frame* frame);

uintptr_t
tm_frame_address(const struct tm_frame* frame);

void*
tm_frame_buffer(const struct tm_frame* frame);

unsigned long
tm_frame_flags(const struct tm_frame* frame);

int
tm_frame_try_lock(struct tm_frame* frame, const void* owner);

void
tm_frame_unlock(struct tm_frame* frame);
