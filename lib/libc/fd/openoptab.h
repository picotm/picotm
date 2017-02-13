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

#ifndef OPENOPTAB_H
#define OPENOPTAB_H

unsigned long
openoptab_append(struct openop **tab, size_t *nelems, int unlink);

void
openoptab_clear(struct openop **tab, size_t *nelems);

void
openoptab_dump(struct openop *tab, size_t nelems);

#endif

