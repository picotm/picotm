/* Copyright (C) 2009  Thomas Zimmermann
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

