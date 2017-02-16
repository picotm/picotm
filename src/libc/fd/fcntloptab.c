/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <stdio.h>
#include "types.h"
#include "range.h"
#include "table.h"
#include "fcntlop.h"
#include "fcntloptab.h"

unsigned long
fcntloptab_append(struct fcntlop **tab, size_t *nelems, int command,
                                                        const union com_fd_fcntl_arg *value,
                                                        const union com_fd_fcntl_arg *oldvalue)
{
    assert(tab);
    assert(nelems);

    void *tmp = tabresize(*tab, *nelems, (*nelems)+1, sizeof((*tab)[0]));

    if (!tmp) {
        return -1;
    }
    *tab = tmp;

    if (fcntlop_init((*tab)+(*nelems), command, value, oldvalue) < 0) {
        return -1;
    }

    return (*nelems)++;
}

void
fcntloptab_clear(struct fcntlop **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    free(*tab);
    *tab = NULL;
    *nelems = 0;
}

void
fcntloptab_dump(struct fcntlop *tab, size_t nelems)
{
    struct fcntlop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        fcntlop_dump(ent);
        fprintf(stderr, "\n");
    }
}

