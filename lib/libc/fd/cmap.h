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

#ifndef CMAP_H
#define CMAP_H

struct cmap_page
{
    pthread_mutex_t    lock;
    unsigned long long count[PGTREE_NENTRIES];
};

int
cmap_page_lock(struct cmap_page *pg);

void
cmap_page_unlock(struct cmap_page *pg);

struct cmap
{
    struct pgtree super;
};

int
cmap_init(struct cmap *cmap);

void
cmap_uninit(struct cmap *cmap);

struct cmap_page *
cmap_lookup_page(struct cmap *cmap, unsigned long long offset);

#endif

