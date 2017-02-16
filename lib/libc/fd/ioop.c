/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "range.h"
#include "ioop.h"

static int
rangeisect(off_t off0, size_t siz0,
           off_t off1, size_t siz1, off_t *off, size_t *siz)
{
    off_t ioff, iend;

    assert(off);
    assert(siz);

    /* Size is zero; no intersection */
    if (!siz0 || !siz1) {
        return 0;
    }

    if ((off0 < off1) && (off0+siz0 > off1)) {
        ioff = off1;
        iend = off1+siz1 < off0+siz0 ? off1+siz1-1 : off0+siz0-1;
    } else if ((off1 < off0) && (off1+siz1 > off0)) {
        ioff = off0;
        iend = off0+siz0 < off1+siz1 ? off0+siz0-1 : off1+siz1-1;
    } else if (off0 == off1) {
        ioff = off0;
        iend = off0 + llmin(siz0, siz1)-1;
    } else {
        return 0;
    }

    *siz = iend-ioff+1;
    *off = ioff;

    return 1;
}

int
ioop_init(struct ioop *ioop, off_t off, size_t nbyte, size_t bufoff)
{
    assert(ioop);

    ioop->off = off;
    ioop->nbyte = nbyte;
    ioop->bufoff = bufoff;

    return 0;
}

void
ioop_uninit(struct ioop *ioop)
{
    assert(ioop);
}

ssize_t
ioop_read(const struct ioop *ioop, void *buf, size_t nbyte,
                                              off_t offset, const void *iobuf)
{
    assert(ioop);
    assert(buf || nbyte);

    ssize_t len;
    off_t  ioff = 0;
    size_t isiz = 0;

    if (rangeisect(offset, nbyte, ioop->off, ioop->nbyte, &ioff, &isiz)) {
        memcpy((char*)buf+ioff-offset, (const char*)iobuf+ioop->bufoff+(ioff-ioop->off), isiz);
        len = ioff+isiz-offset;
    } else {
        len = 0;
    }

    return len;
}

#include <stdio.h>

void
ioop_dump(const struct ioop *ioop)
{
    assert(ioop);

    fprintf(stderr, "ioop %ld %ld %ld", (long)ioop->off,
                                        (long)ioop->nbyte,
                                        (long)ioop->bufoff);
}

int
ioop_uninit_walk(void *ioop)
{
    ioop_uninit(ioop);

    return 1;
}

