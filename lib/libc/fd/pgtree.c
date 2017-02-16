/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <tanger-stm-internal-errcode.h>
#include "pgtree.h"

/*
 * directory entries
 */

static int
pgtree_dir_entry_init(struct pgtree_dir_entry *entry)
{
    int err;

    assert(entry);

    if ( (err = pthread_spin_init(&entry->lock, PTHREAD_PROCESS_PRIVATE)) ) {
        errno = err;
        return ERR_SYSTEM;
    }

    entry->data.dir = NULL;

    return 0;
}

static void
pgtree_dir_entry_uninit(struct pgtree_dir_entry *entry)
{
    pthread_spin_destroy(&entry->lock);
}

/*
 * directories
 */

static int
pgtree_dir_init(struct pgtree_dir *dir)
{
    struct pgtree_dir_entry *entry;
    int err;

    assert(dir);

    err = 0;

    for (entry = dir->entry;
         entry < dir->entry+sizeof(dir->entry)/sizeof(dir->entry[0]);
       ++entry) {
        err = pgtree_dir_entry_init(entry);
    }

    return err;
}

static void
pgtree_dir_uninit(struct pgtree_dir *dir)
{
    struct pgtree_dir_entry *entry;

    assert(dir);

    for (entry = dir->entry;
         entry < dir->entry+sizeof(dir->entry)/sizeof(dir->entry[0]);
       ++entry) {
        pgtree_dir_entry_uninit(entry);
    }
}

/*
 * page tree
 */

static void
pgtree_dir_entry_recursive_cleanup(struct pgtree_dir_entry *entry,
                                   size_t ndirs,
                                   void (*pg_destroy)(void*))
{
    assert(entry);
    assert(pg_destroy);

    if (entry->data.any) {
        if (ndirs) {
            --ndirs;

            struct pgtree_dir_entry *beg = entry->data.dir->entry;
            const struct pgtree_dir_entry *end =
                beg+sizeof(entry->data.dir->entry)/sizeof(entry->data.dir->entry[0]);

            while (beg < end) {
                pgtree_dir_entry_recursive_cleanup(beg, ndirs, pg_destroy);
                ++beg;
            }

            pgtree_dir_uninit(entry->data.dir);
            free(entry->data.dir);
        } else {
            pg_destroy(entry->data.any);
        }
    }
}

int
pgtree_init(struct pgtree *pgtree)
{
    assert(pgtree);

    pgtree->ndirs = 0;

    return pgtree_dir_entry_init(&pgtree->entry);
}

void
pgtree_uninit(struct pgtree *pgtree, void (*pg_destroy)(void*))
{
    assert(pgtree);

    pgtree_dir_entry_recursive_cleanup(&pgtree->entry,
                                        pgtree->ndirs, pg_destroy);
}

void *
pgtree_lookup_page(struct pgtree *pgtree,
                   unsigned long long offset, void* (*pg_create)(void))
{
    assert(pgtree);

    /* grow directory hierarchy */

    unsigned long long offset_prefix = offset>>(pgtree->ndirs*PGTREE_ENTRY_NBITS);

    if (offset_prefix) {

        pthread_spin_lock(&pgtree->entry.lock);

        offset_prefix = offset>>(pgtree->ndirs*PGTREE_ENTRY_NBITS);

        while (offset_prefix) {

            struct pgtree_dir *dir = malloc(sizeof(*dir));

            if (!dir || (pgtree_dir_init(dir) < 0)) {
                free(dir);
                pthread_spin_unlock(&pgtree->entry.lock);
                return NULL;
            }

            dir->entry[0].data.any = pgtree->entry.data.any;

            pgtree->entry.data.dir = dir;

            ++pgtree->ndirs;

            offset_prefix = offset>>(pgtree->ndirs*PGTREE_ENTRY_NBITS);
        }

        pthread_spin_unlock(&pgtree->entry.lock);
    }

    /* map traversal */

    struct pgtree_dir_entry *entry = &pgtree->entry;

    pthread_spin_lock(&entry->lock);

    size_t ndirs = pgtree->ndirs;

    while (ndirs) {

        /* walk down tree up to page level */

        if (!entry->data.dir) {

            /* insert missing directory */

            entry->data.dir = malloc(sizeof(*entry->data.dir));

            if (!entry->data.dir || (pgtree_dir_init(entry->data.dir) < 0)) {
                free(entry->data.dir);
                return NULL;
            }
        }

        unsigned int i = (offset>>(ndirs*PGTREE_ENTRY_NBITS)) & PGTREE_ENTRY_MASK;

        struct pgtree_dir_entry *next_entry = entry->data.dir->entry+i;

        pthread_spin_lock(&next_entry->lock);
        pthread_spin_unlock(&entry->lock);

        entry = next_entry;

        --ndirs;
    }

    /* page lookup */

    if (!entry->data.any) {

        /* insert missing page */

        if (!entry->data.any && !(entry->data.any = pg_create()) ) {
            return NULL;
        }
    }

    void *pg = entry->data.any;

    pthread_spin_unlock(&entry->lock);

    return pg;
}

