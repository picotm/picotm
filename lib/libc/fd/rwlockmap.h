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

#ifndef RWLOCKMAP_H
#define RWLOCKMAP_H

struct rwlockmap_page
{
    unsigned long lock[PGTREE_NENTRIES];
};

struct rwlockmap
{
    struct pgtree super;
};

int
rwlockmap_init(struct rwlockmap *rwlockmap);

void
rwlockmap_uninit(struct rwlockmap *rwlockmap);

struct rwlockmap_page *
rwlockmap_lookup_page(struct rwlockmap *rwlockmap, unsigned long long offset);

#endif

