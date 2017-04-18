/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

