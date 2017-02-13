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

#ifndef REGION_H
#define REGION_H

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

