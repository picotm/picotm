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

#include <assert.h>
#include <stdlib.h>
#include <tanger-stm-ext-actions.h>
#include "table.h"
#include "comalloc.h"

int
com_alloc_init(struct com_alloc *comalloc)
{
    assert(comalloc);

    comalloc->ptrtab = NULL;
    comalloc->ptrtablen = 0;
    comalloc->ptrtabsiz = 0;

    return 0;
}

void
com_alloc_uninit(struct com_alloc *comalloc)
{
    assert(comalloc);

    free(comalloc->ptrtab);
}

int
com_alloc_inject(struct com_alloc *comalloc, enum com_alloc_call call, void *ptr)
{
    assert(comalloc);

    if (__builtin_expect(comalloc->ptrtablen >= comalloc->ptrtabsiz, 0)) {
        void *tmp = tabresize(comalloc->ptrtab,
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

    if (tanger_stm_inject_event(COMPONENT_ALLOC, call, comalloc->ptrtablen) < 0) {
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

