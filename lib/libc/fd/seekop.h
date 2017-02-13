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

#ifndef SEEKOP_H
#define SEEKOP_H

struct seekop
{
    off_t from;
    off_t offset;
    int   whence;
};

int
seekop_init(struct seekop *seekop, off_t from, off_t offset, int whence);

void
seekop_dump(struct seekop *seekop);

#endif

