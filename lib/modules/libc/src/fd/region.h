/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef REGION_H
#define REGION_H

#include <sys/types.h>

struct region
{
    size_t nbyte;
    off_t  offset;
};

int
region_init(struct region *region, size_t nbyte, off_t offset);

void
region_uninit(struct region *region);

void
region_dump(const struct region *region);

#endif

