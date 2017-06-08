/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rwlockmap.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <stdlib.h>
#include <string.h>

/*
 * rwlockmap pages
 */

static int
rwlockmap_page_init(struct rwlockmap_page *pg)
{
    assert(pg);

    atomic_ulong* beg = picotm_arraybeg(pg->lock);
    atomic_ulong* end = picotm_arrayend(pg->lock);

    while (beg < end) {
        atomic_init(beg, 0);
        ++beg;
    }

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

static void*
rwlockmap_create_page_fn(struct picotm_error* error)
{
    struct rwlockmap_page* pg = malloc(sizeof(*pg));
    if (!pg) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    int res = rwlockmap_page_init(pg);
    if (res < 0) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
        goto err_rwlockmap_page_init;
    }

    return pg;

err_rwlockmap_page_init:
    free(pg);
    return NULL;
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

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    pgtree_init(&rwlockmap->super, &error);
    if (picotm_error_is_set(&error)) {
        return -1;
    }
    return 0;
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

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    struct rwlockmap_page* pg = pgtree_lookup_page(&rwlockmap->super, offset,
                                                   rwlockmap_create_page_fn,
                                                   &error);
    if (picotm_error_is_set(&error)) {
        return NULL;
    }
    return pg;
}

