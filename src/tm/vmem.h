/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <pthread.h>
#include <stddef.h>
#include <stdint.h>

struct tm_frame;

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
