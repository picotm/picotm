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

#ifndef REGIONTAB_H
#define REGIONTAB_H

unsigned long
regiontab_append(struct region **tab, size_t *nelems,
                                      size_t *siz,
                                      size_t nbyte,
                                      off_t offset);

void
regiontab_clear(struct region **tab, size_t *nelems);

int
regiontab_sort(struct region *tab, size_t nelems);

void
regiontab_dump(struct region *tab, size_t nelems);

#endif

