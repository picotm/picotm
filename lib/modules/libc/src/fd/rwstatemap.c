/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rwstatemap.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "errcode.h"
#include "pgtree.h"
#include "range.h"
#include "rwlockmap.h"

/*
 * rwstate_page
 */

enum {
    RWSTATE_WRITTEN = 0x8000000,
    RWSTATE_COUNTER = 0x7ffffff
};

struct rwstatemap_page
{
    struct rwlockmap_page *lockpg;
    unsigned long          state[PGTREE_NENTRIES];
};

static int
rwstatemap_page_init(struct rwstatemap_page *statepg)
{
    assert(statepg);

    statepg->lockpg = NULL;
    memset(statepg->state, 0, sizeof(statepg->state));

    return 0;
}

static void
rwstatemap_page_uninit(struct rwstatemap_page *statepg)
{
    assert(statepg);

    return;
}

static struct rwlockmap_page *
rwstatemap_page_get_global_page(struct rwstatemap_page *statepg,
                                unsigned long long offset,
                                struct rwlockmap *rwlockmap)
{
    assert(statepg);

    if (!statepg->lockpg) {
        statepg->lockpg = rwlockmap_lookup_page(rwlockmap, offset);
    }

    return statepg->lockpg;
}

static int
rwstatemap_page_unlock_regions(struct rwstatemap_page *statepg,
                               unsigned long long offset,
                               struct rwlockmap *rwlockmap)
{
    struct rwlockmap_page *lockpg;
    unsigned long long pgoffset;

    assert(statepg);

    lockpg = rwstatemap_page_get_global_page(statepg, offset, rwlockmap);

    if (!lockpg) {
        return ERR_SYSTEM;
    }

    for (pgoffset = 0; pgoffset < PGTREE_NENTRIES; ++pgoffset) {

        if (!(statepg->state[pgoffset]&RWSTATE_COUNTER)) {
            continue;
        }

        unsigned long oldlock, newlock;

        do {
            oldlock = atomic_load(lockpg->lock + pgoffset);
            assert(oldlock&RWSTATE_COUNTER);
            newlock = (oldlock&RWSTATE_COUNTER) - 1;
        } while (!atomic_compare_exchange_strong_explicit(lockpg->lock + pgoffset,
                                                          &oldlock,
                                                          newlock,
                                                          memory_order_acq_rel,
                                                          memory_order_acquire));

        statepg->state[pgoffset] = 0;
    }

    return 0;
}

/*
 * rwstate map
 */

int
rwstatemap_init(struct rwstatemap *rwstatemap)
{
    assert(rwstatemap);

    return pgtreess_init(&rwstatemap->super);
}

static void
rwstatemap_destroy_page_fn(void *statepg)
{
    rwstatemap_page_uninit(statepg);
    free(statepg);
}

void
rwstatemap_uninit(struct rwstatemap *rwstatemap)
{
    assert(rwstatemap);

    pgtreess_uninit(&rwstatemap->super, rwstatemap_destroy_page_fn);
}

static void *
rwstatemap_create_page_fn(void)
{
    struct rwstatemap_page *statepg = malloc(sizeof(*statepg));

    if (!statepg || (rwstatemap_page_init(statepg) < 0)) {
        free(statepg);
        return NULL;
    }

    return statepg;
}

int
rwstatemap_rdlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap)
{
    unsigned long long lockedlength, lockedoffset;
    int err;

    lockedlength = 0;
    lockedoffset = offset;

    assert(rwstatemap);

    err = 0;

    while (length && !err) {

        struct rwstatemap_page *statepg = pgtreess_lookup_page(&rwstatemap->super,
                                                                offset,
                                                                rwstatemap_create_page_fn);

        if (!statepg) {
            return ERR_SYSTEM;
        }

        struct rwlockmap_page *lockpg =
            rwstatemap_page_get_global_page(statepg, offset, rwlockmap);

        if (!lockpg) {
            return ERR_SYSTEM;
        }

        /* compute next slice */

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        length -= diff;
        offset += diff;

        /* read-lock records */

        while (diff && !err) {

            if ( !(statepg->state[pgoffset]&RWSTATE_COUNTER) ) {
                /* not yet locked */

                unsigned long oldlock, newlock;

                do {
                    oldlock = lockpg->lock[pgoffset];

                    if (oldlock&RWSTATE_WRITTEN) {
                        /* concurrent writer present */
                        err = ERR_CONFLICT;
                        goto doreturn;
                    }

                    newlock = (oldlock&RWSTATE_COUNTER) + 1;

                } while (!atomic_compare_exchange_strong_explicit(lockpg->lock + pgoffset,
                                                                  &oldlock,
                                                                  newlock,
                                                                  memory_order_acq_rel,
                                                                  memory_order_acquire));
            }

            statepg->state[pgoffset] =
                (statepg->state[pgoffset]&RWSTATE_WRITTEN) |
               ((statepg->state[pgoffset]&RWSTATE_COUNTER) + 1);

            ++lockedlength;
            ++pgoffset;
            --diff;
        }
    }

    doreturn:

    if (err) {
        /* unlock all locked records if error occured */
        rwstatemap_unlock(rwstatemap, lockedlength, lockedoffset, rwlockmap);
    }

    return err;
}

int
rwstatemap_wrlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap)
{
    unsigned long long lockedlength, lockedoffset;
    int err;

    assert(rwstatemap);

    err = 0;

    lockedlength = 0;
    lockedoffset = offset;

    while (length && !err) {

        struct rwstatemap_page *statepg =
            pgtreess_lookup_page(&rwstatemap->super, offset,
                                  rwstatemap_create_page_fn);

        if (!statepg) {
            return ERR_SYSTEM;
        }

        struct rwlockmap_page *lockpg =
            rwstatemap_page_get_global_page(statepg, offset, rwlockmap);

        if (!lockpg) {
            return ERR_SYSTEM;
        }

        /* compute next slice */

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        length -= diff;
        offset += diff;

        /* write-lock records */

        while (diff && !err) {

            if ( !(statepg->state[pgoffset]&RWSTATE_COUNTER) ) {
                /* not yet locked */

                unsigned long oldlock, newlock;

                do {
                    oldlock = lockpg->lock[pgoffset];

                    if (oldlock&RWSTATE_WRITTEN) {
                        /* concurrent writer present */
                        err = ERR_CONFLICT;
                        goto doreturn;
                    } else if ( (oldlock&RWSTATE_COUNTER) > 1) {
                        /* concurrent readers present */
                        err = ERR_CONFLICT;
                        goto doreturn;
                    }

                    newlock = RWSTATE_WRITTEN | ((oldlock&RWSTATE_COUNTER)+1);

                } while (!atomic_compare_exchange_strong_explicit(lockpg->lock + pgoffset,
                                                                  &oldlock,
                                                                  newlock,
                                                                  memory_order_acq_rel,
                                                                  memory_order_acquire));

            } else if ( !(statepg->state[pgoffset]&RWSTATE_WRITTEN) ) {

                unsigned long oldlock, newlock;

                do {
                    oldlock = lockpg->lock[pgoffset];

                    /* only read-locked */

                    if (oldlock&RWSTATE_WRITTEN) {
                        /* concurrent writer present */
                        err = ERR_CONFLICT;
                        goto doreturn;
                    } else if ( (oldlock&RWSTATE_COUNTER) > 1) {
                        /* concurrent readers present */
                        err = ERR_CONFLICT;
                        goto doreturn;
                    }

                    newlock = oldlock | RWSTATE_WRITTEN;

                } while (!atomic_compare_exchange_strong_explicit(lockpg->lock + pgoffset,
                                                                  &oldlock,
                                                                  newlock,
                                                                  memory_order_acq_rel,
                                                                  memory_order_acquire));
            }

            statepg->state[pgoffset] = RWSTATE_WRITTEN
                | ((statepg->state[pgoffset]&RWSTATE_COUNTER) + 1);

            ++lockedlength;
            ++pgoffset;
            --diff;
        }
    }

    doreturn:

    if (err) {
        /* unlock all locked records if error occured */
        rwstatemap_unlock(rwstatemap, lockedlength, lockedoffset, rwlockmap);
    }

    return err;
}

int
rwstatemap_unlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap)
{
    assert(rwstatemap);

    while (length) {

        struct rwstatemap_page *statepg
            = pgtreess_lookup_page(&rwstatemap->super,
                                    offset,
                                    rwstatemap_create_page_fn);

        if (!statepg) {
            return ERR_SYSTEM;
        }

        struct rwlockmap_page *lockpg =
            rwstatemap_page_get_global_page(statepg, offset, rwlockmap);

        if (!lockpg) {
            return ERR_SYSTEM;
        }

        /* compute next slice */

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        length -= diff;
        offset += diff;

        /* unlock records */

        while (diff) {

            assert(statepg->state[pgoffset]&RWSTATE_COUNTER);

            statepg->state[pgoffset] =
                (statepg->state[pgoffset]&RWSTATE_WRITTEN) |
               ((statepg->state[pgoffset]&RWSTATE_COUNTER) - 1);

            if ( !(statepg->state[pgoffset]&RWSTATE_COUNTER) ) {
                /* last lock of this transaction */

                if (statepg->state[pgoffset]&RWSTATE_WRITTEN) {

                    unsigned long oldlock, newlock;

                    do {
                        oldlock = lockpg->lock[pgoffset];
                        newlock = (oldlock&RWSTATE_COUNTER) - 1;
                    } while (!atomic_compare_exchange_strong_explicit(lockpg->lock + pgoffset,
                                                                      &oldlock,
                                                                      newlock,
                                                                      memory_order_acq_rel,
                                                                      memory_order_acquire));
                } else {

                    unsigned long oldlock, newlock;

                    do {
                        oldlock = lockpg->lock[pgoffset];

                        newlock = (oldlock&RWSTATE_WRITTEN)
                                | ((oldlock&RWSTATE_COUNTER) - 1);

                    } while (!atomic_compare_exchange_strong_explicit(lockpg->lock + pgoffset,
                                                                      &oldlock,
                                                                      newlock,
                                                                      memory_order_acq_rel,
                                                                      memory_order_acquire));
                }

                statepg->state[pgoffset] = 0;
            }

            ++pgoffset;
            --diff;
        }
    }

    return 0;
}

static int
rwstatemap_for_each_page_unlock_regions(void *statepg,
                                        unsigned long long offset,
                                        void *rwlockmap)
{
    return rwstatemap_page_unlock_regions(statepg, offset, rwlockmap);
}

int
rwstatemap_unlock_all(struct rwstatemap *rwstatemap,
                      struct rwlockmap *rwlockmap)
{
    return pgtreess_for_each_page(&rwstatemap->super,
                                   rwstatemap_for_each_page_unlock_regions,
                                   rwlockmap);
}

