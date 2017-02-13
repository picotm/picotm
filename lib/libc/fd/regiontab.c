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
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include "range.h"
#include "table.h"
#include "region.h"
#include "regiontab.h"

unsigned long
regiontab_append(struct region **tab, size_t *nelems,
                                      size_t *siz,
                                      size_t nbyte,
                                      off_t offset)
{
    assert(tab);
    assert(nelems);

    if (__builtin_expect(*nelems >= *siz, 0)) {
        void *tmp = tabresize(*tab, *siz, (*siz)+1, sizeof((*tab)[0]));

        if (!tmp) {
            return -1;
        }
        *tab = tmp;

        ++(*siz);
    }

    if (region_init((*tab)+(*nelems), nbyte, offset) < 0) {
        return -1;
    }

    return (*nelems)++;
}

int
region_uninit_walk(void *region)
{
    region_uninit(region);
    return 1;
}

void
regiontab_clear(struct region **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    tabwalk_1(*tab, *nelems, sizeof((*tab)[0]), region_uninit_walk);

    free(*tab);
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

