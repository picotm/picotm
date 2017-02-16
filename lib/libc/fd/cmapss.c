/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <tanger-stm-internal-errcode.h>
#include "range.h"
#include "pgtree.h"
#include "cmap.h"
#include "pgtreess.h"
#include "cmapss.h"

/*
 * snapshot page
 */

enum {
    CMAPSS_PAGE_ISFILLED = 1<<0
};

struct cmapss_page
{
    struct cmap_page  *pg; /* reference to global page */
    unsigned long      nlocks;
    unsigned long      flags;
    unsigned long long count[PGTREE_NENTRIES];
};

static int
cmapss_page_init(struct cmapss_page *sspg)
{
    assert(sspg);

    sspg->pg = NULL;
    sspg->nlocks = 0;
    sspg->flags = 0;
    memset(sspg->count, 0, sizeof(sspg->count));

    return 0;
}

static void
cmapss_page_uninit(struct cmapss_page *sspg)
{
    assert(sspg);
}

static struct cmap_page *
cmapss_page_get_global_page(struct cmapss_page *sspg,
                            unsigned long long offset,
                            struct cmap *cmap)
{
    assert(sspg);

    if (__builtin_expect(!sspg->pg, 0)) {
        sspg->pg = cmap_lookup_page(cmap, offset);
    }

    return sspg->pg;
}

static void
cmapss_page_clear_flags(struct cmapss_page *sspg)
{
    assert(sspg);

    sspg->flags = 0;
}

/*
 * version snapshot
 */

int
cmapss_init(struct cmapss *cmapss)
{
    assert(cmapss);

    return pgtreess_init(&cmapss->super);
}

static void
cmapss_destroy_page_fn(void *sspg)
{
    cmapss_page_uninit(sspg);
    free(sspg);
}

void
cmapss_uninit(struct cmapss *cmapss)
{
    assert(cmapss);

    pgtreess_uninit(&cmapss->super, cmapss_destroy_page_fn);
}

int
cmapss_for_each_page_clear_flags_fn(void *sspg,
                                    unsigned long long offset,
                                    void *data)
{
    cmapss_page_clear_flags(sspg);

    return 0;
}

void
cmapss_clear(struct cmapss *cmapss)
{
    pgtreess_for_each_page(&cmapss->super,
                            cmapss_for_each_page_clear_flags_fn, NULL);
}

static void *
cmapss_create_page_fn(void)
{
    struct cmapss_page *sspg = malloc(sizeof(*sspg));

    if (!sspg || (cmapss_page_init(sspg) < 0)) {
        free(sspg);
        return NULL;
    }

    return sspg;
}

int
cmapss_get_region(struct cmapss *cmapss, size_t length,
                                         off_t offset, struct cmap *cmap)
{
    assert(cmapss);

    if (!length) {
        return 0;
    }

    /* extend region to cover complete pages */

    unsigned long long beg =  offset / PGTREE_NENTRIES;
    unsigned long long end = (offset+length) / PGTREE_NENTRIES;

    offset =  beg * PGTREE_NENTRIES;
    length = (end-beg+1) * PGTREE_NENTRIES;

    /* retrieve page versions from global table */

    while (length) {

        struct cmapss_page *sspg = pgtreess_lookup_page(&cmapss->super,
                                                         offset,
                                                         cmapss_create_page_fn);

        if (!sspg) {
            return ERR_SYSTEM;
        }

        /* compute slice */

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        if (!(sspg->flags&CMAPSS_PAGE_ISFILLED)) {

            int err;

            /* read global record versions */

            struct cmap_page *pg =
                cmapss_page_get_global_page(sspg, offset, cmap);

            if (!pg) {
                return ERR_SYSTEM;
            }

            /* not yet locked */
            if ((err = cmap_page_lock(pg)) < 0) {
                return err;
            }

            memcpy(sspg->count+pgoffset,
                     pg->count+pgoffset, diff*sizeof(sspg->count[0]));

            /* unlock */
            cmap_page_unlock(pg);

            sspg->flags |= CMAPSS_PAGE_ISFILLED;
        }

        length -= diff;
        offset += diff;
    }

    return 0;
}

int
cmapss_validate_region(struct cmapss *cmapss, size_t length,
                                              off_t offset, struct cmap *cmap)
{
    int valid = 1;

    assert(cmapss);

    while (length && valid) {

        struct cmapss_page *sspg = pgtreess_lookup_page(&cmapss->super,
                                                         offset,
                                                         cmapss_create_page_fn);

        if (!sspg) {
            return ERR_SYSTEM;
        }

        assert(sspg->flags&CMAPSS_PAGE_ISFILLED);

        struct cmap_page *pg =
            cmapss_page_get_global_page(sspg, offset, cmap);

        if (!pg) {
            return ERR_SYSTEM;
        }

        /* compute slice */

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        length -= diff;
        offset += diff;

        /* validate */
        if (!sspg->nlocks) {
            int err;
            if ((err = cmap_page_lock(pg)) < 0) { /* not yet locked */
                return err;
            }
        }

        while (diff && valid) {
            valid = sspg->count[pgoffset] >= pg->count[pgoffset];
            ++pgoffset;
            --diff;
        }

        if (!sspg->nlocks) {
            cmap_page_unlock(pg); /* unlock if necessary */
        }
    }

    return valid ? 0 : ERR_CONFLICT;
}

int
cmapss_inc_region(struct cmapss *cmapss, size_t length,
                                         off_t offset, struct cmap *cmap)
{
    assert(cmapss);

    while (length) {

        struct cmapss_page *sspg = pgtreess_lookup_page(&cmapss->super,
                                                         offset,
                                                         cmapss_create_page_fn);

        if (!sspg) {
            return ERR_SYSTEM;
        }

        /* the local page does not have to be filled, e.g. when writing only */

        struct cmap_page *pg =
            cmapss_page_get_global_page(sspg, offset, cmap);

        if (!pg) {
            return ERR_SYSTEM;
        }

        /* compute slice */

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        length -= diff;
        offset += diff;

        /* increment version numbers */

        if (!sspg->nlocks) {
            int err;
            if ((err = cmap_page_lock(pg)) < 0) { /* not yet locked */
                return err;
            }
        }

        while (diff) {
            ++pg->count[pgoffset];
            ++pgoffset;
            --diff;
        }

        if (!sspg->nlocks) {
            cmap_page_unlock(pg); /* unlock if necessary */
        }
    }

    return 0;
}

int
cmapss_lock_region(struct cmapss *cmapss, size_t length,
                                          off_t offset, struct cmap *cmap)
{
    assert(cmapss);

    while (length) {

        struct cmapss_page *sspg = pgtreess_lookup_page(&cmapss->super,
                                                         offset,
                                                         cmapss_create_page_fn);

        if (!sspg) {
            return ERR_SYSTEM;
        }

        if (!sspg->nlocks) {

            /* lock global page */

            struct cmap_page *pg =
                cmapss_page_get_global_page(sspg, offset, cmap);

            if (!pg) {
                return ERR_SYSTEM;
            }

            int err = cmap_page_lock(pg);

            if (err < 0) {
                return err;
            }
        }

        /* increment local lock counter */
        ++sspg->nlocks;

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        length -= diff;
        offset += diff;
    }

    return 0;
}

int
cmapss_unlock_region(struct cmapss *cmapss, size_t length,
                                            off_t offset, struct cmap *cmap)
{
    assert(cmapss);

    while (length) {

        struct cmapss_page *sspg = pgtreess_lookup_page(&cmapss->super,
                                                         offset,
                                                         cmapss_create_page_fn);

        if (!sspg) {
            return ERR_SYSTEM;
        }

        assert(sspg->nlocks);

        /* decrement local local counter */
        --sspg->nlocks;

        if (!sspg->nlocks) {

            /* unlock global page */

            struct cmap_page *pg =
                cmapss_page_get_global_page(sspg, offset, cmap);

            if (!pg) {
                return ERR_SYSTEM;
            }

            cmap_page_unlock(pg);
        }

        unsigned long long pgoffset = offset%PGTREE_NENTRIES;
        unsigned long long diff = llmin(length, PGTREE_NENTRIES-pgoffset);

        length -= diff;
        offset += diff;
    }

    return 0;
}

