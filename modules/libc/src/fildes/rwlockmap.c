/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "rwlockmap.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
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
