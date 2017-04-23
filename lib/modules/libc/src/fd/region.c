/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

