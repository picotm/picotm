/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "allocator_tx.h"
#include <assert.h>
#include <stdlib.h>
#include <picotm/picotm-module.h>

/**
 * Round size up to next multiple of word size.
 */
static size_t
rnd2wb(size_t size)
{
    const unsigned int mask = sizeof(void*) - 1;

    return (size + mask) & ~mask;
}

int
allocator_tx_init(struct allocator_tx *self, unsigned long module)
{
    assert(self);

    self->module = module;

    self->ptrtab = NULL;
    self->ptrtablen = 0;
    self->ptrtabsiz = 0;

    return 0;
}

void
allocator_tx_uninit(struct allocator_tx *self)
{
    assert(self);

    picotm_tabfree(self->ptrtab);
    self->ptrtab = NULL;
    self->ptrtablen = 0;
    self->ptrtabsiz = 0;
}

int
allocator_tx_inject(struct allocator_tx *self, enum allocator_tx_call call, void *ptr)
{
    assert(self);

    if (__builtin_expect(self->ptrtablen >= self->ptrtabsiz, 0)) {
        void *tmp = picotm_tabresize(self->ptrtab,
                                    self->ptrtabsiz,
                                    self->ptrtabsiz+1,
                                    sizeof(self->ptrtab[0]));
        if (!tmp) {
            return -1;
        }
        self->ptrtab = tmp;

        ++self->ptrtabsiz;
    }

    void **ptrtab = self->ptrtab+self->ptrtablen;

    *ptrtab = ptr;

    if (picotm_inject_event(self->module, call, self->ptrtablen) < 0) {
        return -1;
    }

    return self->ptrtablen++;
}

int
allocator_tx_apply_event(struct allocator_tx *self, const struct event *event, size_t n)
{
    static int (* const apply_func[LAST_ACTION])(struct allocator_tx*, unsigned int) = {
        allocator_tx_apply_posix_memalign,
        allocator_tx_apply_free};

    assert(event || !n);
    assert(event->call < LAST_ACTION);

    int err = 0;

    while (n && !err) {
        err = apply_func[event->call](self, event->cookie);
        --n;
        ++event;
    }

    return err;
}

int
allocator_tx_undo_event(struct allocator_tx *self, const struct event *event, size_t n)
{
    static int (* const undo_func[LAST_ACTION])(struct allocator_tx*, unsigned int) = {
        allocator_tx_undo_posix_memalign,
        allocator_tx_undo_free};

    assert(event || !n);
    assert(event->call < LAST_ACTION);

    int err = 0;

    event += n;

    while (n && !err) {
        --event;
        err = undo_func[event->call](self, event->cookie);
        --n;
    }

    return err;
}

void
allocator_tx_finish(struct allocator_tx *self)
{
    assert(self);

    self->ptrtablen = 0;
}

/*
 * free()
 */

void
allocator_tx_exec_free(struct allocator_tx *self, void *mem)
{
    assert(self);

    /* Inject event */
    if (allocator_tx_inject(self, ACTION_FREE, mem) < 0) {
        return;
    }
}

int
allocator_tx_apply_free(struct allocator_tx *self, unsigned int cookie)
{
    assert(self);
    assert(cookie < self->ptrtablen);

    free(self->ptrtab[cookie]);

    return 0;
}

int
allocator_tx_undo_free(struct allocator_tx *self, unsigned int cookie)
{
    assert(self);
    assert(cookie < self->ptrtablen);

    return 0;
}

/*
 * posix_memalign()
 */

int
allocator_tx_exec_posix_memalign(struct allocator_tx *self, void **memptr, size_t alignment, size_t size)
{
    assert(self);

    /* Allocate memory */

    if (posix_memalign(memptr, alignment, rnd2wb(size)) < 0) {
        return -1;
    }

    /* Inject event */
    if (allocator_tx_inject(self, ACTION_POSIX_MEMALIGN, *memptr) < 0) {
        free(*memptr);
        return -1;
    }

    return 0;
}

int
allocator_tx_apply_posix_memalign(struct allocator_tx *self, unsigned int cookie)
{
    assert(self);
    assert(cookie < self->ptrtablen);

    return 0;
}

int
allocator_tx_undo_posix_memalign(struct allocator_tx *self, unsigned int cookie)
{
    assert(self);
    assert(cookie < self->ptrtablen);

    free(self->ptrtab[cookie]);

    return 0;
}
