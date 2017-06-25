/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#include "ioop.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "range.h"

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

    if ((off0 < off1) && ((off_t)(off0+siz0) > off1)) {
        ioff = off1;
        iend = off1+siz1 < off0+siz0 ? off1+siz1-1 : off0+siz0-1;
    } else if ((off1 < off0) && ((off_t)(off1+siz1) > off0)) {
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

void
ioop_init(struct ioop *ioop, off_t off, size_t nbyte, size_t bufoff)
{
    assert(ioop);

    ioop->off = off;
    ioop->nbyte = nbyte;
    ioop->bufoff = bufoff;
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
