/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMALLOC_H
#define COMALLOC_H

#include <stddef.h>

struct event;

enum com_alloc_call
{
    ACTION_POSIX_MEMALIGN = 0,
    ACTION_FREE,
    LAST_ACTION
};

struct com_alloc {
    unsigned long module;

    void **ptrtab;
    size_t ptrtablen;
    size_t ptrtabsiz;
};

int
com_alloc_init(struct com_alloc *comalloc, unsigned long module);

void
com_alloc_uninit(struct com_alloc *comalloc);

int
com_alloc_inject(struct com_alloc *comalloc, enum com_alloc_call call, void *ptr);

int
com_alloc_apply_event(struct com_alloc *comalloc, const struct event *event, size_t n);

int
com_alloc_undo_event(struct com_alloc *comalloc, const struct event *event, size_t n);

void
com_alloc_finish(struct com_alloc *comalloc);

/*
 * Execute methods
 */

int
com_alloc_exec_posix_memalign(struct com_alloc *comalloc, void **, size_t, size_t);

void
com_alloc_exec_free(struct com_alloc *comalloc, void *ptr);

/*
 * Apply methods
 */

int
com_alloc_apply_posix_memalign(struct com_alloc *comalloc, unsigned int cookie);

int
com_alloc_apply_free(struct com_alloc *comalloc, unsigned int cookie);

/*
 * Undo methods
 */

int
com_alloc_undo_posix_memalign(struct com_alloc *comalloc, unsigned int cookie);

int
com_alloc_undo_free(struct com_alloc *comalloc, unsigned int cookie);

#endif

