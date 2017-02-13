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
#include <sys/types.h>
#include "region.h"

int
region_init(struct region *region, size_t nbyte, off_t offset)
{
    assert(region);

    region->nbyte = nbyte;
    region->offset = offset;

    return 0;
}

void
region_uninit(struct region *region)
{
    assert(region);

    return;
}

#include <stdio.h>

void
region_dump(const struct region *region)
{
    assert(region);

    fprintf(stderr, "region %ld %ld", (long)region->nbyte,
                                      (long)region->offset);
}

