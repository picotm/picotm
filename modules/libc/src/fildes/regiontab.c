/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
