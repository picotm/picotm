/*
 * picotm - A system-level transaction manager
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

#include "ioop.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "range.h"

static int
rangeisect(off_t off0, size_t siz0,
           off_t off1, size_t siz1, off_t* off, size_t* siz)
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
        iend = off1 + siz1 < off0 + siz0 ? off1 + siz1 - 1 : off0 + siz0 - 1;
    } else if ((off1 < off0) && ((off_t)(off1 + siz1) > off0)) {
        ioff = off0;
        iend = off0 + siz0 < off1 + siz1 ? off0 + siz0 - 1 : off1 + siz1 - 1;
    } else if (off0 == off1) {
        ioff = off0;
        iend = off0 + llmin(siz0, siz1) - 1;
    } else {
        return 0;
    }

    *siz = iend - ioff + 1;
    *off = ioff;

    return 1;
}

void
ioop_init(struct ioop* self, off_t off, size_t nbyte, size_t bufoff)
{
    assert(self);

    self->off = off;
    self->nbyte = nbyte;
    self->bufoff = bufoff;
}

void
ioop_uninit(struct ioop* self)
{
    assert(self);
}

ssize_t
ioop_read(const struct ioop* self, void* buf, size_t nbyte, off_t offset,
          const void* iobuf)
{
    assert(self);
    assert(buf || nbyte);

    ssize_t len;
    off_t ioff = 0;
    size_t isiz = 0;

    if (rangeisect(offset, nbyte, self->off, self->nbyte, &ioff, &isiz)) {
        memcpy((char*)buf + ioff-offset,
               (const char*)iobuf + self->bufoff + (ioff - self->off),
               isiz);
        len = ioff + isiz - offset;
    } else {
        len = 0;
    }

    return len;
}
