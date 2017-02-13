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
#include "comalloc.h"

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

