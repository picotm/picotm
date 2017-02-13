/* Copyright (C) 2009  Thomas Zimmermann
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
#include <stdio.h>
#include <sys/types.h>
#include "seekop.h"

int
seekop_init(struct seekop *seekop, off_t from, off_t offset, int whence)
{
    assert(seekop);

    seekop->from = from;
    seekop->offset = offset;
    seekop->whence = whence;

    return 0;
}

void
seekop_dump(struct seekop *seekop)
{
    fprintf(stderr, "seekop %p %ld %ld %d", (void*)seekop, (long)seekop->from,
                                                           (long)seekop->offset,
                                                                 seekop->whence);
}

