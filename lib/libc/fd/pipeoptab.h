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

#ifndef PIPEOPTAB_H
#define PIPEOPTAB_H

unsigned long
pipeoptab_append(struct pipeop **tab, size_t *nelems, int pipefd[2]);

void
pipeoptab_clear(struct pipeop **tab, size_t *nelems);

void
pipeoptab_dump(struct pipeop *tab, size_t nelems);

#endif

