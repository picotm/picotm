/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "table.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Based on http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 */
static size_t
rndpow2(size_t val)
{
    --val;

    for (size_t i = 1; i < (sizeof(val) * CHAR_BIT); i *= 2) {
        val |= val >> i;
    }

    ++val;

    return val;
}

void*
tabresize(void* restrict base, size_t nelems, size_t newnelems, size_t siz)
{
    static bool s_lowmem = false;

    void* mem;

do_alloc:

    if (!s_lowmem) {

        const size_t nelems2 = rndpow2(nelems);
        const size_t newnelems2 = rndpow2(newnelems);

        if (nelems2 != newnelems2) {
            mem = realloc(base, newnelems2 * siz);
        } else {
            mem = base;
        }
    } else {
        mem = realloc(base, newnelems * siz);
    }

    if (!mem && newnelems) {
        if (errno == ENOMEM && !s_lowmem) {
            s_lowmem = true;
            goto do_alloc; /* Try with linear-growth algorithm */
        }
    }

    return mem;
}

void
tabfree(void* restrict base)
{
    free(base);
}

int
tabwalk_1(void* restrict base, size_t nelems, size_t siz, int (*walk)(void*))
{
    unsigned char* beg = base;
    const unsigned char* end = beg + nelems * siz;

    int inc;

    for (inc = 1; (inc > 0) && (beg < end); beg += inc * siz) {
        inc = walk(beg);
    }

    return inc < 0 ? inc : 0;
}

int
tabwalk_2(void* restrict base, size_t nelems, size_t siz,
          int (*walk)(void*, void*), void* data)
{
    unsigned char* beg = base;
    const unsigned char* end = beg + nelems * siz;

    int inc;

    for (inc = 1; (inc > 0) && (beg < end); beg += inc * siz) {
        inc = walk(beg, data);
    }

    return inc < 0 ? inc : 0;
}

int
tabwalk_3(void* restrict base, size_t nelems, size_t siz,
          int (*walk)(void*, void*, void*), void* data1, void* data2)
{
    unsigned char* beg = base;
    const unsigned char* end = beg + nelems * siz;

    int inc;

    for (inc = 1; (inc > 0) && (beg < end); beg += inc * siz) {
        inc = walk(beg, data1, data2);
    }

    return inc < 0 ? inc : 0;
}

int
tabrwalk_1(void* restrict base, size_t nelems, size_t siz,
           int (*walk)(void*))
{
    const unsigned char* beg = base;
    unsigned char* end = base + nelems * siz;

    int dec;

    for (dec = 1; (dec > 0) && (end > beg);) {
        end -= dec * siz;
        dec = walk(end);
    }

    return dec < 0 ? dec : 0;
}

int
tabrwalk_2(void* restrict base, size_t nelems, size_t siz,
           int (*walk)(void*, void*), void* data)
{
    const unsigned char* beg = base;
    unsigned char* end = base + nelems * siz;

    int dec;

    for (dec = 1; (dec > 0) && (end > beg);) {
        end -= dec * siz;
        dec = walk(end, data);
    }

    return dec < 0 ? dec : 0;
}

size_t
tabuniq(void* restrict base, size_t nelems, size_t siz,
        int (*compare)(const void*, const void*))
{
    size_t uniqelems = 0;

    unsigned char* beg = base;
    const unsigned char* end = beg + nelems * siz;
    const unsigned char* next = beg;

    while (next < end) {

        do {
            next += siz;
        } while ((next < end) && !compare(beg, next));

        if (next != end) {

            const ptrdiff_t diff = next - beg;

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
