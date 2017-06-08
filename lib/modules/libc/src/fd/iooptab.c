/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "iooptab.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include "range.h"
#include "ioop.h"

unsigned long
iooptab_append(struct ioop * restrict * restrict tab, size_t * restrict nelems,
                                                      size_t * restrict siz,
                                                      size_t nbyte,
                                                      off_t offset,
                                                      size_t bufoff)
{
    assert(tab);
    assert(nelems);

    if (__builtin_expect(*nelems >= *siz, 0)) {

        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        void *tmp = picotm_tabresize(*tab, *siz, (*siz)+1, sizeof((*tab)[0]),
                                     &error);
        if (picotm_error_is_set(&error)) {
            return -1;
        }
        *tab = tmp;

        ++(*siz);
    }

    if (ioop_init((*tab)+(*nelems), offset, nbyte, bufoff) < 0) {
        return -1;
    }

    return (*nelems)++;
}

static size_t
ioop_uninit_walk(void *ioop, struct picotm_error* error)
{
    ioop_uninit(ioop);
    return 1;
}

void
iooptab_clear(struct ioop * restrict * restrict tab, size_t * restrict nelems)
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
iooptab_read(struct ioop * restrict tab, size_t nelems,
                                         void * restrict buf,
                                         size_t nbyte,
                                         off_t offset,
                                         void * restrict iobuf)
{
    struct ioop *ioop;
    ssize_t len = 0;

    for (ioop = tab; ioop < tab+nelems; ++ioop) {
        len = llmax(len, ioop_read(ioop, buf, nbyte, offset, iobuf));
    }

    return len;
}

int
ioopcmp(const struct ioop *lhs, const struct ioop *rhs)
{
    assert(lhs);
    assert(rhs);

//    return (lhs->off < rhs->offset) ? -1 : (lhs->off > rhs->offset ? 1 : 0)

    return ((long long)lhs->off) - ((long long)rhs->off);
}

int
ioopcmp_compare(const void *lhs, const void *rhs)
{
    return ioopcmp(lhs, rhs);
}

int
iooptab_sort(const struct ioop * restrict tab, size_t nelems,
                   struct ioop * restrict * restrict sorted)
{
    assert(tab);
    assert(sorted);

    void *tmp = realloc(*sorted, nelems*sizeof(**sorted));

    if (!tmp) {
        return -1;
    }
    *sorted = tmp;

    memcpy(*sorted, tab, nelems*sizeof(**sorted));

    qsort(*sorted, nelems, sizeof(**sorted), ioopcmp_compare);

    return 0;
}

void
iooptab_dump(struct ioop * restrict tab, size_t nelems)
{
    struct ioop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        ioop_dump(ent);
        fprintf(stderr, "\n");
    }
}

