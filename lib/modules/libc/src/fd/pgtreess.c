/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "errcode.h"
#include "range.h"
#include "pgtree.h"
#include "pgtreess.h"

static long long
raisell(long long base, long long exp)
{
    long long res = 1;

    while (exp) {
        res *= base;
        --exp;
    }

    return res;
}

/*
 * snapshot directory
 */

struct pgtreess_dir
{
    union pgtreess_dir_entry entry[PGTREE_NENTRIES];
};

static int
pgtreess_dir_init(struct pgtreess_dir *ssdir)
{
    assert(ssdir);

    memset(ssdir->entry, 0, sizeof(ssdir->entry));

    return 0;
}

static void
pgtreess_dir_uninit(struct pgtreess_dir *ssdir)
{
    assert(ssdir);
}

/*
 * version snapshot
 */
int
pgtreess_init(struct pgtreess *pgtreess)
{
    assert(pgtreess);

    pgtreess->ndirs = 0;
    pgtreess->entry.any = NULL;

    return 0;
}

static void
pgtreess_dir_entry_recursive_cleanup(union pgtreess_dir_entry *entry,
                                     size_t ndirs,
                                     void (*destroy_page_fn)(void*))
{
    assert(entry);

    if (entry->any) {
        if (ndirs) {
            --ndirs;

            union pgtreess_dir_entry *beg = entry->dir->entry;
            const union pgtreess_dir_entry *end =
                beg+sizeof(entry->dir->entry)/sizeof(entry->dir->entry[0]);

            while (beg < end) {
                pgtreess_dir_entry_recursive_cleanup(beg,
                                                     ndirs,
                                                     destroy_page_fn);
                ++beg;
            }

            pgtreess_dir_uninit(entry->dir);
            free(entry->dir);
        } else {
            destroy_page_fn(entry->any);
        }
    }
}

void
pgtreess_uninit(struct pgtreess *pgtreess, void (*destroy_page_fn)(void*))
{
    assert(pgtreess);

    pgtreess_dir_entry_recursive_cleanup(&pgtreess->entry,
                                          pgtreess->ndirs, destroy_page_fn);
}

struct dir_stack
{
    union pgtreess_dir_entry entry;
    size_t                   i;
    unsigned long long       offset;
    size_t                   depth;
};

void *
pgtreess_lookup_page(struct pgtreess *pgtreess,
                      unsigned long long offset, void* (*create_page_fn)(void))
{
    assert(pgtreess);

    /* insert root directories */

    unsigned long long offset_prefix =
        offset >> (pgtreess->ndirs*PGTREE_ENTRY_NBITS);

    while (offset_prefix) {
        struct pgtreess_dir *ssdir = malloc(sizeof(*ssdir));

        if (!ssdir || (pgtreess_dir_init(ssdir) < 0)) {
            free(ssdir);
            return NULL;
        }
        ssdir->entry->any = pgtreess->entry.any;

        pgtreess->entry.any = ssdir;
        ++pgtreess->ndirs;

        offset_prefix = offset >> (pgtreess->ndirs*PGTREE_ENTRY_NBITS);
    }

    /* walk through tree */

    unsigned long long ndirs = pgtreess->ndirs;
    union pgtreess_dir_entry *entry = &pgtreess->entry;

    while (ndirs && entry->dir) {

        unsigned int i =
            (offset>>(ndirs*PGTREE_ENTRY_NBITS)) & PGTREE_ENTRY_MASK;

        entry = entry->dir->entry+i;
        --ndirs;
    }

    /* insert leaf directories */

    while (ndirs) {
        entry->dir = malloc(sizeof(*entry->dir));

        if (!entry->dir || (pgtreess_dir_init(entry->dir) < 0)) {
            free(entry->dir);
            return NULL;
        }

        unsigned int i =
            (offset>>(ndirs*PGTREE_ENTRY_NBITS))&PGTREE_ENTRY_MASK;

        entry = entry->dir->entry+i;
        --ndirs;
    }

    /* lookup page */

    if (!entry->any) {
        entry->any = create_page_fn();
    }

    return entry->any;
}

int
pgtreess_for_each_page(struct pgtreess *pgtreess,
                       int (*page_fn)(void*, unsigned long long, void*),
                       void *data)
{
    struct dir_stack *stack;
    size_t depth;

    assert(pgtreess);
    assert(page_fn);

    /* allocate enough stack to hold n directories plus 1 page */

    stack = malloc((pgtreess->ndirs+1)*sizeof(*stack));

    if (!stack) {
        return ERR_SYSTEM;
    }

    depth = 0;

    if (pgtreess->entry.any) {
        stack[depth].entry = pgtreess->entry;
        stack[depth].i = 0;
        stack[depth].offset = 0;
        stack[depth].depth = 0;
        ++depth;
    }

    /* recure through map and clear pages in order of appearrance */

    while (depth) {

        struct dir_stack *top = stack+depth-1;

        if (top->depth < pgtreess->ndirs) {
            /* directory */

            while ((top->i < PGTREE_NENTRIES) && !top->entry.dir->entry[top->i].any) {
                ++top->i;
            }

            if (top->i < PGTREE_NENTRIES) {

                /* max number of page elements below this directory */
                long long pwr =
                    raisell(PGTREE_NENTRIES, pgtreess->ndirs-top->depth);

                stack[depth].entry = top->entry.dir->entry[top->i];
                stack[depth].i = 0;
                stack[depth].offset = top->offset+top->i*pwr;
                stack[depth].depth = top->depth+1;
                ++top->i;
                ++depth;
            } else {
                /* do nothing */
                --depth;
            }
        } else {
            /* page */

            int err = page_fn(top->entry.any, top->offset, data);

            if (err) {
                return err;
            }

            --depth;
        }
    }

    /* cleanup */
    free(stack);

    return 0;
}

