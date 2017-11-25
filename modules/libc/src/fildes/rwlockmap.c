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
#include <sys/types.h>

/*
 * rwlockmap pages
 */

static void
rwlockmap_page_init(struct rwlockmap_page* self)
{
    assert(self);

    struct picotm_rwlock* beg = picotm_arraybeg(self->lock);
    const struct picotm_rwlock* end = picotm_arrayend(self->lock);

    while (beg < end) {
        picotm_rwlock_init(beg);
        ++beg;
    }
}

static void
rwlockmap_page_uninit(struct rwlockmap_page* self)
{
    assert(self);
}

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

/*
 * rwlockmap
 */

static void
rwlockmap_destroy_page_fn(uintptr_t value,
                          struct picotm_shared_treemap* treemap)
{
    struct rwlockmap_page* pg = (struct rwlockmap_page*)value;
    rwlockmap_page_uninit(pg);
    free(pg);
}

/* The number of bits in an offset that are handled by the
 * treemap. The lowest bits in each offset index into the
 * page. The higher bits index into the treemap directories.
 */
static unsigned long
key_nbits(unsigned long page_nbits)
{
    return (sizeof(off_t) * CHAR_BIT) - RWLOCKMAP_PAGE_NBITS;
}

static unsigned long long
key_bits(unsigned long long offset, unsigned long page_nbits)
{
    return offset >> page_nbits;
}

void
rwlockmap_init(struct rwlockmap* self)
{
    assert(self);

    picotm_shared_treemap_init(&self->pagemap,
                               key_nbits(RWLOCKMAP_PAGE_NBITS),
                               RWLOCKMAP_PAGE_NBITS);
}

void
rwlockmap_uninit(struct rwlockmap* self)
{
    assert(self);

    picotm_shared_treemap_uninit(&self->pagemap, rwlockmap_destroy_page_fn);
}

struct rwlockmap_page*
rwlockmap_find_page(struct rwlockmap* self, unsigned long long record_offset,
                    struct picotm_error* error)
{
    assert(self);

    uintptr_t value = picotm_shared_treemap_find_value(
        &self->pagemap, key_bits(record_offset, RWLOCKMAP_PAGE_NBITS),
        rwlockmap_create_page_fn,
        rwlockmap_destroy_page_fn, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return (struct rwlockmap_page*)value;
}
