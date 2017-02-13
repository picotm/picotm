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
#include <string.h>
#include <search.h>
#include "table.h"
#include "openop.h"
#include "openoptab.h"

unsigned long
openoptab_append(struct openop **tab, size_t *nelems, int unlink)
{
    assert(tab);
    assert(nelems);

    void *tmp = tabresize(*tab, *nelems, (*nelems)+1, sizeof((*tab)[0]));

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

    free(*tab);
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

