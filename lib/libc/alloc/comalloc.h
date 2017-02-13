/* Copyright (C) 2008-2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef COMALLOC_H
#define COMALLOC_H

enum com_alloc_call
{
    ACTION_POSIX_MEMALIGN = 0,
    ACTION_FREE,
    LAST_ACTION
};

struct com_alloc
{
    void **ptrtab;
    size_t ptrtablen;
    size_t ptrtabsiz;
};

int
com_alloc_init(struct com_alloc *comalloc);

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

