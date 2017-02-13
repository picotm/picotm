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

