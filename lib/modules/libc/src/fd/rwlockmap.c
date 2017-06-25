/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

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

static void
rwlockmap_page_init(struct rwlockmap_page *pg)
{
    assert(pg);

    atomic_ulong* beg = picotm_arraybeg(pg->lock);
    atomic_ulong* end = picotm_arrayend(pg->lock);

    while (beg < end) {
        atomic_init(beg, 0);
        ++beg;
    }
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

    rwlockmap_page_init(pg);

    return pg;
}

static void
rwlockmap_destroy_page_fn(void *pg)
{
    rwlockmap_page_uninit(pg);
    free(pg);
}

void
rwlockmap_init(struct rwlockmap* rwlockmap, struct picotm_error* error)
{
    assert(rwlockmap);

    pgtree_init(&rwlockmap->super, error);
}

void
rwlockmap_uninit(struct rwlockmap *rwlockmap)
{
    assert(rwlockmap);

    pgtree_uninit(&rwlockmap->super, rwlockmap_destroy_page_fn);
}

struct rwlockmap_page*
rwlockmap_lookup_page(struct rwlockmap* rwlockmap, unsigned long long offset,
                      struct picotm_error* error)
{
    assert(rwlockmap);

    return pgtree_lookup_page(&rwlockmap->super, offset,
                              rwlockmap_create_page_fn, error);
}
