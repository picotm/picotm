/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "regiontab.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>
#include "region.h"

unsigned long
regiontab_append(struct region **tab, size_t *nelems,
                                      size_t *siz,
                                      size_t nbyte,
                                      off_t offset,
                                      struct picotm_error* error)
{
    assert(tab);
    assert(nelems);

    if (__builtin_expect(*nelems >= *siz, 0)) {

        void *tmp = picotm_tabresize(*tab, *siz, (*siz)+1, sizeof((*tab)[0]),
                                     error);
        if (picotm_error_is_set(error)) {
            return (unsigned long)-1;
        }
        *tab = tmp;

        ++(*siz);
    }

    region_init((*tab)+(*nelems), nbyte, offset);

    return (*nelems)++;
}

static size_t
region_uninit_walk(void* region, struct picotm_error* error)
{
    region_uninit(region);
    return 1;
}

void
regiontab_clear(struct region **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(*tab, *nelems, sizeof((*tab)[0]), region_uninit_walk,
                     &error);

    picotm_tabfree(*tab);
    *tab = NULL;
    *nelems = 0;
}

static int
regioncmp(const struct region *lhs, const struct region *rhs)
{
    assert(lhs);
    assert(rhs);

//    return (lhs->off < rhs->offset) ? -1 : (lhs->off > rhs->offset ? 1 : 0)

    return ((long long)lhs->offset) - ((long long)rhs->offset);
}

static int
regioncmp_compare(const void *lhs, const void *rhs)
{
    return regioncmp(lhs, rhs);
}

int
regiontab_sort(struct region *tab, size_t nelems)
{
    qsort(tab, nelems, sizeof(*tab), regioncmp_compare);

    return 0;
}

#include <stdio.h>

void
regiontab_dump(struct region *tab, size_t nelems)
{
    struct region *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        region_dump(ent);
        fprintf(stderr, "\n");
    }
}

