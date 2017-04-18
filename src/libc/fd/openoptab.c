/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <picotm/picotm-module.h>
#include "openop.h"
#include "openoptab.h"

unsigned long
openoptab_append(struct openop **tab, size_t *nelems, int unlink)
{
    assert(tab);
    assert(nelems);

    void *tmp = picotm_tabresize(*tab, *nelems, (*nelems)+1, sizeof((*tab)[0]));

    if (!tmp) {
        return -1;
    }
    *tab = tmp;

    if (openop_init((*tab)+(*nelems), unlink) < 0) {
        return -1;
    }

    return (*nelems)++;
}

void
openoptab_clear(struct openop **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    picotm_tabfree(*tab);
    *tab = NULL;
    *nelems = 0;
}

#include <stdio.h>

void
openoptab_dump(struct openop *tab, size_t nelems)
{
    struct openop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        openop_dump(ent);
        fprintf(stderr, "\n");
    }
}

