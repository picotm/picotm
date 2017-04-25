/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comalloc.h"
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
com_alloc_init(struct com_alloc *comalloc, unsigned long module)
{
    assert(comalloc);

    comalloc->module = module;

    comalloc->ptrtab = NULL;
    comalloc->ptrtablen = 0;
    comalloc->ptrtabsiz = 0;

    return 0;
}

void
com_alloc_uninit(struct com_alloc *comalloc)
{
    assert(comalloc);

    picotm_tabfree(comalloc->ptrtab);
    comalloc->ptrtab = NULL;
    comalloc->ptrtablen = 0;
    comalloc->ptrtabsiz = 0;
}

int
com_alloc_inject(struct com_alloc *comalloc, enum com_alloc_call call, void *ptr)
{
    assert(comalloc);

    if (__builtin_expect(comalloc->ptrtablen >= comalloc->ptrtabsiz, 0)) {
        void *tmp = picotm_tabresize(comalloc->ptrtab,
                                    comalloc->ptrtabsiz,
                                    comalloc->ptrtabsiz+1,
                                    sizeof(comalloc->ptrtab[0]));
        if (!tmp) {
            return -1;
        }
        comalloc->ptrtab = tmp;

        ++comalloc->ptrtabsiz;
    }

    void **ptrtab = comalloc->ptrtab+comalloc->ptrtablen;

    *ptrtab = ptr;

    if (picotm_inject_event(comalloc->module, call, comalloc->ptrtablen) < 0) {
        return -1;
    }

    return comalloc->ptrtablen++;
}

int
com_alloc_apply_event(struct com_alloc *comalloc, const struct event *event, size_t n)
{
    static int (* const apply_func[LAST_ACTION])(struct com_alloc*, unsigned int) = {
        com_alloc_apply_posix_memalign,
        com_alloc_apply_free};

    assert(event || !n);
    assert(event->call < LAST_ACTION);

    int err = 0;

    while (n && !err) {
        err = apply_func[event->call](comalloc, event->cookie);
        --n;
        ++event;
    }

    return err;
}

int
com_alloc_undo_event(struct com_alloc *comalloc, const struct event *event, size_t n)
{
    static int (* const undo_func[LAST_ACTION])(struct com_alloc*, unsigned int) = {
        com_alloc_undo_posix_memalign,
        com_alloc_undo_free};

    assert(event || !n);
    assert(event->call < LAST_ACTION);

    int err = 0;

    event += n;

    while (n && !err) {
        --event;
        err = undo_func[event->call](comalloc, event->cookie);
        --n;
    }

    return err;
}

void
com_alloc_finish(struct com_alloc *comalloc)
{
    assert(comalloc);

    comalloc->ptrtablen = 0;
}

/*
 * free()
 */

void
com_alloc_exec_free(struct com_alloc *comalloc, void *mem)
{
    assert(comalloc);

    /* Inject event */
    if (com_alloc_inject(comalloc, ACTION_FREE, mem) < 0) {
        return;
    }
}

int
com_alloc_apply_free(struct com_alloc *comalloc, unsigned int cookie)
{
    assert(comalloc);
    assert(cookie < comalloc->ptrtablen);

    free(comalloc->ptrtab[cookie]);

    return 0;
}

int
com_alloc_undo_free(struct com_alloc *comalloc, unsigned int cookie)
{
    assert(comalloc);
    assert(cookie < comalloc->ptrtablen);

    return 0;
}

/*
 * posix_memalign()
 */

int
com_alloc_exec_posix_memalign(struct com_alloc *comalloc, void **memptr, size_t alignment, size_t size)
{
    assert(comalloc);

    /* Allocate memory */

    if (posix_memalign(memptr, alignment, rnd2wb(size)) < 0) {
        return -1;
    }

    /* Inject event */
    if (com_alloc_inject(comalloc, ACTION_POSIX_MEMALIGN, *memptr) < 0) {
        free(*memptr);
        return -1;
    }

    return 0;
}

int
com_alloc_apply_posix_memalign(struct com_alloc *comalloc, unsigned int cookie)
{
    assert(comalloc);
    assert(cookie < comalloc->ptrtablen);

    return 0;
}

int
com_alloc_undo_posix_memalign(struct com_alloc *comalloc, unsigned int cookie)
{
    assert(comalloc);
    assert(cookie < comalloc->ptrtablen);

    free(comalloc->ptrtab[cookie]);

    return 0;
}
