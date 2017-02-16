/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

