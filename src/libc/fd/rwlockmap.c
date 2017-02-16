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
#include "rwlockmap.h"

/*
 * rwlockmap pages
 */

static int
rwlockmap_page_init(struct rwlockmap_page *pg)
{
    assert(pg);

    memset(pg->lock, 0, sizeof(pg->lock));

    return 0;
}

static void
rwlockmap_page_uninit(struct rwlockmap_page *pg)
{
    assert(pg);
}

/*
 * rwlockmap
 */

static void *
rwlockmap_create_page_fn(void)
{
    struct rwlockmap_page *pg = malloc(sizeof(*pg));

    if (!pg || (rwlockmap_page_init(pg) < 0)) {
        free(pg);
        return NULL;
    }

    return pg;
}

static void
rwlockmap_destroy_page_fn(void *pg)
{
    rwlockmap_page_uninit(pg);
    free(pg);
}

int
rwlockmap_init(struct rwlockmap *rwlockmap)
{
    assert(rwlockmap);

    return pgtree_init(&rwlockmap->super);
}

void
rwlockmap_uninit(struct rwlockmap *rwlockmap)
{
    assert(rwlockmap);

    pgtree_uninit(&rwlockmap->super, rwlockmap_destroy_page_fn);
}

struct rwlockmap_page *
rwlockmap_lookup_page(struct rwlockmap *rwlockmap, unsigned long long offset)
{
    assert(rwlockmap);

    return pgtree_lookup_page(&rwlockmap->super, offset,
                               rwlockmap_create_page_fn);
}

