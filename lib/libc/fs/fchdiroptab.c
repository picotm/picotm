/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <stdio.h>
#include "table.h"
#include "fchdirop.h"
#include "fchdiroptab.h"

unsigned long
fchdiroptab_append(struct fchdirop **tab, size_t *nelems, int oldcwd,
                                                          int newcwd)
{
    assert(tab);
    assert(nelems);

    void *tmp = tabresize(*tab, *nelems, (*nelems)+1, sizeof((*tab)[0]));

    if (!tmp) {
        return -1;
    }
    *tab = tmp;

    if (fchdirop_init((*tab)+(*nelems), oldcwd, newcwd) < 0) {
        return -1;
    }

    return (*nelems)++;
}

void
fchdiroptab_clear(struct fchdirop **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    free(*tab);
    *tab = NULL;
    *nelems = 0;
}

void
fchdiroptab_dump(struct fchdirop *tab, size_t nelems)
{
    struct fchdirop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        fchdirop_dump(ent);
        fprintf(stderr, "\n");
    }
}

