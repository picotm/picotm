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
#include <stdio.h>
#include "table.h"
#include "pipeop.h"
#include "pipeoptab.h"

unsigned long
pipeoptab_append(struct pipeop **tab, size_t *nelems, int pipefd[2])
{
    assert(tab);
    assert(nelems);

    void *tmp = tabresize(*tab, *nelems, (*nelems)+1, sizeof((*tab)[0]));

    if (!tmp) {
        return -1;
    }
    *tab = tmp;

    if (pipeop_init((*tab)+(*nelems), pipefd) < 0) {
        return -1;
    }

    return (*nelems)++;
}

void
pipeoptab_clear(struct pipeop **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    free(*tab);
    *tab = NULL;
    *nelems = 0;
}

void
pipeoptab_dump(struct pipeop *tab, size_t nelems)
{
    struct pipeop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        pipeop_dump(ent);
        fprintf(stderr, "\n");
    }
}

