/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cmap.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "errcode.h"

/*
 * cmap pages
 */

static int
cmap_page_init(struct cmap_page *pg)
{
    int err;

    assert(pg);

    if ( (err = pthread_mutex_init(&pg->lock, NULL)) ) {
        return -1;
    }

    memset(pg->count, 0, sizeof(pg->count));

    return 0;
}

static void
cmap_page_uninit(struct cmap_page *pg)
{
    assert(pg);

    pthread_mutex_destroy(&pg->lock);
}

int
cmap_page_lock(struct cmap_page *pg)
{
    assert(pg);

    int err = pthread_mutex_lock(&pg->lock);

    if (__builtin_expect(err, 0)) {
        errno = err;
        return ERR_SYSTEM;
    }

    return 0;
}

void
cmap_page_unlock(struct cmap_page *pg)
{
    assert(pg);

    int err = pthread_mutex_unlock(&pg->lock);

    if (__builtin_expect(err, 0)) {
        abort();
    }
}

/*
 * cmap
 */

static void *
cmap_create_page_fn(void)
{
    struct cmap_page *pg = malloc(sizeof(*pg));

    if (!pg || (cmap_page_init(pg) < 0)) {
        free(pg);
        return NULL;
    }

    return pg;
}

static void
cmap_destroy_page_fn(void *pg)
{
    cmap_page_uninit(pg);
    free(pg);
}

int
cmap_init(struct cmap *cmap)
{
    assert(cmap);

    return pgtree_init(&cmap->super);
}

void
cmap_uninit(struct cmap *cmap)
{
    assert(cmap);

    pgtree_uninit(&cmap->super, cmap_destroy_page_fn);
}

struct cmap_page *
cmap_lookup_page(struct cmap *cmap, unsigned long long offset)
{
    assert(cmap);

    return pgtree_lookup_page(&cmap->super, offset, cmap_create_page_fn);
}

