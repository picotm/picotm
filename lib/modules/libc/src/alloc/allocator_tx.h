/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>

struct event;

enum allocator_tx_call
{
    ACTION_POSIX_MEMALIGN = 0,
    ACTION_FREE,
    LAST_ACTION
};

struct allocator_tx {
    unsigned long module;

    void **ptrtab;
    size_t ptrtablen;
    size_t ptrtabsiz;
};

int
allocator_tx_init(struct allocator_tx *self, unsigned long module);

void
allocator_tx_uninit(struct allocator_tx *self);

int
allocator_tx_inject(struct allocator_tx *self, enum allocator_tx_call call, void *ptr);

int
allocator_tx_apply_event(struct allocator_tx *self, const struct event *event, size_t n);

int
allocator_tx_undo_event(struct allocator_tx *self, const struct event *event, size_t n);

void
allocator_tx_finish(struct allocator_tx *self);

/*
 * Execute methods
 */

int
allocator_tx_exec_posix_memalign(struct allocator_tx *self, void **, size_t, size_t);

void
allocator_tx_exec_free(struct allocator_tx *self, void *ptr);

/*
 * Apply methods
 */

int
allocator_tx_apply_posix_memalign(struct allocator_tx *self, unsigned int cookie);

int
allocator_tx_apply_free(struct allocator_tx *self, unsigned int cookie);

/*
 * Undo methods
 */

int
allocator_tx_undo_posix_memalign(struct allocator_tx *self, unsigned int cookie);

int
allocator_tx_undo_free(struct allocator_tx *self, unsigned int cookie);
