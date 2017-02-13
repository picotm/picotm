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

#ifndef PGTREE_H
#define PGTREE_H

enum {
    PGTREE_ENTRY_NBITS = 9,
    PGTREE_ENTRY_MASK  = 0x1ff,
    PGTREE_NENTRIES    = (0x1 << PGTREE_ENTRY_NBITS)
};

struct pgtree_dir;

struct pgtree_dir_entry
{
    pthread_spinlock_t  lock;
    union {
        struct pgtree_dir *dir;
        void              *any;
    } data;
};

struct pgtree_dir
{
    struct pgtree_dir_entry entry[PGTREE_NENTRIES];
};

struct pgtree
{
    size_t                  ndirs;
    struct pgtree_dir_entry entry;
};

int
pgtree_init(struct pgtree *pgtree);

void
pgtree_uninit(struct pgtree *pgtree, void (*pg_destroy_fn)(void*));

void *
pgtree_lookup_page(struct pgtree *pgtree,
                   unsigned long long offset, void* (*pg_create_fn)(void));

#endif

