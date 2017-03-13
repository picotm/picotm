/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

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

/**
 * |struct tm_vmem| represents main memory; the resource that
 * the TM module maintains.
 */
struct tm_vmem {
    pthread_rwlock_t lock; /**< \brief Structure lock */

    /* frame-allocator fields */
    struct tm_frame* frametab; /**< \brief Table of frames */
    size_t           frametablen; /**< \brief Number of elements in frametab */
};

int
tm_vmem_init(struct tm_vmem* vmem);

void
tm_vmem_uninit(struct tm_vmem* vmem);

struct tm_frame*
tm_vmem_acquire_frame_by_block(struct tm_vmem* vmem, size_t block_index);

struct tm_frame*
tm_vmem_acquire_frame_by_address(struct tm_vmem* vmem, uintptr_t addr);

void
tm_vmem_release_frame(struct tm_vmem* vmem, struct tm_frame* frame);
