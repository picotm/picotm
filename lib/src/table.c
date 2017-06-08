/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "table.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "picotm/picotm-error.h"

/* Rounds up to the next power of 2.
 *
 * Based on http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
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
tabresize(void* restrict base, size_t nelems, size_t newnelems, size_t siz,
          struct picotm_error* error)
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
        picotm_error_set_errno(error, errno);
    }

    return mem;
}

void
tabfree(void* restrict base)
{
    free(base);
}

size_t
tabwalk_1(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, struct picotm_error*),
          struct picotm_error* error)
{
    size_t pos = 0;

    unsigned char* beg = base;
    const unsigned char* end = beg + nelems * siz;

    while (beg < end) {
        size_t inc = walk(beg, error);

        if (picotm_error_is_set(error)) {
            return pos;
        }

        beg += inc * siz;
        pos += inc;
    }

    return pos;
}

size_t
tabwalk_2(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, void*, struct picotm_error*), void* data,
          struct picotm_error* error)
{
    size_t pos = 0;

    unsigned char* beg = base;
    const unsigned char* end = beg + nelems * siz;

    while (beg < end) {
        size_t inc = walk(beg, data, error);

        if (picotm_error_is_set(error)) {
            return pos;
        }

        beg += inc * siz;
        pos += inc;
    }

    return pos;
}

size_t
tabwalk_3(void* restrict base, size_t nelems, size_t siz,
          size_t (*walk)(void*, void*, void*, struct picotm_error*),
          void* data1, void* data2, struct picotm_error* error)
{
    size_t pos = 0;

    unsigned char* beg = base;
    const unsigned char* end = beg + nelems * siz;

    while (beg < end) {
        size_t inc = walk(beg, data1, data2, error);

        if (picotm_error_is_set(error)) {
            return pos;
        }

        beg += inc * siz;
        pos += inc;
    }

    return pos;
}

size_t
tabrwalk_1(void* restrict base, size_t nelems, size_t siz,
           size_t (*walk)(void*, struct picotm_error*),
           struct picotm_error* error)
{
    size_t pos = 0;

    const unsigned char* beg = base;
    unsigned char* end = base + nelems * siz;

    end -= siz;

    while (end >= beg) {
        size_t dec = walk(end, error);

        if (picotm_error_is_set(error)) {
            return pos;
        }

        end -= dec * siz;
        pos += dec;
    }

    return pos;
}

size_t
tabrwalk_2(void* restrict base, size_t nelems, size_t siz,
           size_t (*walk)(void*, void*, struct picotm_error*), void* data,
           struct picotm_error* error)
{
    size_t pos = 0;

    const unsigned char* beg = base;
    unsigned char* end = base + nelems * siz;

    end -= siz;

    while (end >= beg) {
        size_t dec = walk(end, data, error);

        if (picotm_error_is_set(error)) {
            return pos;
        }

        end -= dec * siz;
        pos += dec;
    }

    return pos;
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

            if (diff == (ptrdiff_t)siz) {
                beg += siz;
            } else {
                beg = memcpy(beg+siz, next, siz);
            }
        }

        ++uniqelems;
    }

    return uniqelems;
}
