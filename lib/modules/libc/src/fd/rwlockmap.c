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
#include <limits.h>
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

    struct picotm_rwlock* beg = picotm_arraybeg(pg->lock);
    const struct picotm_rwlock* end = picotm_arrayend(pg->lock);

    while (beg < end) {
        picotm_rwlock_init(beg);
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

static uintptr_t
rwlockmap_create_page_fn(unsigned long long key,
                         struct picotm_shared_treemap* treemap,
                         struct picotm_error* error)
{
    struct rwlockmap_page* pg = malloc(sizeof(*pg));
    if (!pg) {
        picotm_error_set_errno(error, errno);
        return 0;
    }

    rwlockmap_page_init(pg);

    return (uintptr_t)pg;
}

static void
rwlockmap_destroy_page_fn(uintptr_t value,
                          struct picotm_shared_treemap* treemap)
{
    struct rwlockmap_page* pg = (struct rwlockmap_page*)value;
    rwlockmap_page_uninit(pg);
    free(pg);
}

void
rwlockmap_init(struct rwlockmap* rwlockmap, unsigned long record_bits,
               struct picotm_error* error)
{
    assert(rwlockmap);

    /* The number of bits in an offset that are handled by the
     * treemap. The lowest bits in each offset index into the record
     * and page. The higher bits index into the treemap directories.
     */
    unsigned long offset_bits =
        (sizeof(off_t) * CHAR_BIT) - (RWLOCKMAP_PAGE_NBITS + record_bits);

    picotm_shared_treemap_init(&rwlockmap->pagemap, offset_bits,
                               RWLOCKMAP_PAGE_NBITS);
}

void
rwlockmap_uninit(struct rwlockmap *rwlockmap)
{
    assert(rwlockmap);

    picotm_shared_treemap_uninit(&rwlockmap->pagemap,
                                 rwlockmap_destroy_page_fn);
}

struct rwlockmap_page*
rwlockmap_lookup_page(struct rwlockmap* rwlockmap, unsigned long long offset,
                      struct picotm_error* error)
{
    assert(rwlockmap);

    uintptr_t value = picotm_shared_treemap_find_value(
        &rwlockmap->pagemap, offset, rwlockmap_create_page_fn,
        rwlockmap_destroy_page_fn, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return (struct rwlockmap_page*)value;
}
