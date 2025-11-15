/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "regiontab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-tab.h"
#include <assert.h>
#include <stdlib.h>
#include "region.h"

unsigned long
regiontab_append(struct region** tab, size_t* nelems,
                                      size_t* siz,
                                      size_t nbyte,
                                      off_t offset,
                                      struct picotm_error* error)
{
    assert(tab);
    assert(nelems);

    if (__builtin_expect(*nelems >= *siz, 0)) {

        void *tmp = picotm_tabresize(*tab, *siz, (*siz) + 1,
                                     sizeof((*tab)[0]), error);
        if (picotm_error_is_set(error)) {
            return (unsigned long)-1;
        }
        *tab = tmp;

        ++(*siz);
    }

    region_init((*tab) + (*nelems), nbyte, offset);

    return (*nelems)++;
}

static size_t
region_uninit_walk(void* region, struct picotm_error* error)
{
    region_uninit(region);
    return 1;
}

void
regiontab_clear(struct region** tab, size_t* nelems)
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
regioncmp(const struct region* lhs, const struct region* rhs)
{
    assert(lhs);
    assert(rhs);

    return ((long long)lhs->offset) - ((long long)rhs->offset);
}

static int
regioncmp_compare(const void* lhs, const void* rhs)
{
    return regioncmp(lhs, rhs);
}

int
regiontab_sort(struct region* tab, size_t nelems)
{
    qsort(tab, nelems, sizeof(*tab), regioncmp_compare);

    return 0;
}
