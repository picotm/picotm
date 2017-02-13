/* Copyright (C) 2008-2009  Thomas Zimmermann
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
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

/* Based on http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
static size_t
rndpow2(size_t val)
{
    size_t i;

    --val;

    for (i = 1; i < sizeof(val)*8; i*=2) {
        val |= val >> i;
    }

    ++val;

    return val;
}

void *
tabresize(void * restrict base, size_t nelems, size_t newnelems, size_t siz)
{
    static int lowmem = 0;
    void *mem;

    doalloc:

    if (!lowmem) {

        const size_t nelems2 = rndpow2(nelems);
        const size_t newnelems2 = rndpow2(newnelems);

        if (nelems2 != newnelems2) {
            mem = realloc(base, newnelems2*siz);
        } else {
            mem = base;
        }
    } else {
        mem = realloc(base, newnelems*siz);
    }

    if (!mem && newnelems) {
        if (errno == ENOMEM && !lowmem) {
            lowmem = 1;
            goto doalloc; /* Try with linear-complexity algorithm */
        } else {
            perror("realloc");
        }
    }

    return mem;
}

int
tabwalk_1(void * restrict base, size_t nelems, size_t siz, int (*walk)(void*))
{
    assert(walk);

    unsigned char *beg = base;
    unsigned char *end = beg + nelems*siz;

    int inc;

    for (inc = 1; (inc > 0) && (beg < end); beg += inc*siz) {
        inc = walk(beg);
    }

    return inc < 0 ? inc : 0;
}

int
tabwalk_2(void * restrict base, size_t nelems,
                                size_t siz,
                                int (*walk)(void*, void*), void *data)
{
    assert(walk);

    unsigned char *beg = base;
    unsigned char *end = beg + nelems*siz;

    int inc;

    for (inc = 1; (inc > 0) && (beg < end); beg += inc*siz) {
        inc = walk(beg, data);
    }

    return inc < 0 ? inc : 0;
}

int
tabwalk_3(void * restrict base, size_t nelems,
                                size_t siz,
                                int (*walk)(void*, void*, void*), void *data1,
                                                                  void *data2)
{
    assert(walk);

    unsigned char *beg = base;
    unsigned char *end = beg + nelems*siz;

    int inc;

    for (inc = 1; (inc > 0) && (beg < end); beg += inc*siz) {
        inc = walk(beg, data1, data2);
    }

    return inc < 0 ? inc : 0;
}

int
tabrwalk_1(void * restrict base, size_t nelems,
                                 size_t siz, int (*walk)(void*))
{
    assert(walk);

    unsigned char *beg = base;
    unsigned char *end = beg + nelems*siz;

    int dec;

    for (dec = 1; (dec > 0) && (end > beg);) {
        end -= dec*siz;
        dec = walk(end);
    }

    return dec < 0 ? dec : 0;
}

int
tabrwalk_2(void * restrict base, size_t nelems,
                                 size_t siz,
                                 int (*walk)(void*, void*), void *data)
{
    assert(walk);

    unsigned char *beg = base;
    unsigned char *end = beg + nelems*siz;

    int dec;

    for (dec = 1; (dec > 0) && (end > beg);) {
        end -= dec*siz;
        dec = walk(end, data);
    }

    return dec < 0 ? dec : 0;
}

size_t
tabuniq(void * restrict base, size_t nelems,
                              size_t siz,
                              int (*compare)(const void*, const void*))
{
    size_t uniqelems = 0;

    assert(base || !nelems);
    assert(siz);
    assert(compare);

    unsigned char *beg = base;

    const unsigned char *end = beg + nelems*siz;
    const unsigned char *next = beg;

    while (next < end) {

        do {
            next += siz;
        } while ((next < end) && !compare(beg, next));

        if (next != end) {

            const ptrdiff_t diff = next-beg;

            if (diff == siz) {
                beg += siz;
            } else {
                beg = memcpy(beg+siz, next, siz);
            }
        }

        ++uniqelems;
    }

    return uniqelems;
}

