/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pipeoptab.h"
#include <assert.h>
#include <picotm/picotm-lib-tab.h>
#include <stdio.h>
#include "pipeop.h"

unsigned long
pipeoptab_append(struct pipeop **tab, size_t *nelems, int pipefd[2])
{
    assert(tab);
    assert(nelems);

    void *tmp = picotm_tabresize(*tab, *nelems, (*nelems)+1, sizeof((*tab)[0]));

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

    picotm_tabfree(*tab);
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

