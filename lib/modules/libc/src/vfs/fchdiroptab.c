/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "fchdiroptab.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <stdio.h>
#include "fchdirop.h"

unsigned long
fchdiroptab_append(struct fchdirop **tab, size_t *nelems, int oldcwd,
                                                          int newcwd)
{
    assert(tab);
    assert(nelems);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    void *tmp = picotm_tabresize(*tab, *nelems, (*nelems)+1,
                                 sizeof((*tab)[0]), &error);
    if (picotm_error_is_set(&error)) {
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

    picotm_tabfree(*tab);
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

