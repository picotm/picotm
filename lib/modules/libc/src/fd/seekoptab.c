/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "seekoptab.h"
#include <assert.h>
#include <stdio.h>
#include <picotm/picotm-lib-tab.h>
#include "seekop.h"

unsigned long
seekoptab_append(struct seekop **tab, size_t *nelems, off_t from,
                                                      off_t offset, int whence)
{
    assert(tab);
    assert(nelems);

    void *tmp = picotm_tabresize(*tab, *nelems, (*nelems)+1, sizeof((*tab)[0]));

    if (!tmp) {
        return -1;
    }
    *tab = tmp;

    if (seekop_init((*tab)+(*nelems), from, offset, whence) < 0) {
        return -1;
    }

    return (*nelems)++;
}

void
seekoptab_clear(struct seekop **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    picotm_tabfree(*tab);
    *tab = NULL;
    *nelems = 0;
}

void
seekoptab_dump(struct seekop *tab, size_t nelems)
{
    struct seekop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        seekop_dump(ent);
        fprintf(stderr, "\n");
    }
}

