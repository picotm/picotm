/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "iooptab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-tab.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "range.h"
#include "ioop.h"

unsigned long
iooptab_append(struct ioop** tab, size_t* nelems, size_t* siz, size_t nbyte,
               off_t offset, size_t bufoff, struct picotm_error* error)
{
    assert(tab);
    assert(nelems);

    if (__builtin_expect(*nelems >= *siz, 0)) {

        void *tmp = picotm_tabresize(*tab, *siz, *siz + 1, sizeof((*tab)[0]),
                                     error);
        if (picotm_error_is_set(error)) {
            return (unsigned long)-1;
        }
        *tab = tmp;

        ++(*siz);
    }

    ioop_init((*tab) + (*nelems), offset, nbyte, bufoff);

    return (*nelems)++;
}

static size_t
ioop_uninit_walk(void* ioop, struct picotm_error* error)
{
    ioop_uninit(ioop);
    return 1;
}

void
iooptab_clear(struct ioop** tab, size_t* nelems)
{
    assert(tab);
    assert(nelems);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(*tab, *nelems, sizeof((*tab)[0]), ioop_uninit_walk,
                     &error);

    picotm_tabfree(*tab);
    *tab = NULL;
    *nelems = 0;
}

ssize_t
iooptab_read(struct ioop* tab, size_t nelems, void* buf, size_t nbyte,
             off_t offset, void* iobuf)
{
    ssize_t len = 0;

    for (struct ioop* ioop = tab; ioop < tab+nelems; ++ioop) {
        len = llmax(len, ioop_read(ioop, buf, nbyte, offset, iobuf));
    }

    return len;
}

static int
ioopcmp(const struct ioop* lhs, const struct ioop* rhs)
{
    assert(lhs);
    assert(rhs);

    return ((long long)lhs->off) - ((long long)rhs->off);
}

static int
ioopcmp_compare(const void* lhs, const void* rhs)
{
    return ioopcmp(lhs, rhs);
}

void
iooptab_sort(const struct ioop* tab, size_t nelems, struct ioop** sorted,
             struct picotm_error* error)
{
    assert(tab);
    assert(sorted);

    void* tmp = realloc(*sorted, nelems * sizeof(**sorted));
    if (!tmp) {
        picotm_error_set_errno(error, errno);
        return;
    }
    *sorted = tmp;

    memcpy(*sorted, tab, nelems * sizeof(**sorted));

    qsort(*sorted, nelems, sizeof(**sorted), ioopcmp_compare);
}
