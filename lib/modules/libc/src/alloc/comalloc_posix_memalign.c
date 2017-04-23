/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdlib.h>
#include "rnd2wb.h"
#include "comalloc.h"

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

