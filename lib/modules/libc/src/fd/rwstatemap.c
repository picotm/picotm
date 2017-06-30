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

#include "rwstatemap.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <stdlib.h>
#include <string.h>
#include "range.h"
#include "rwlockmap.h"

/*
 * rwstate_page
 */

enum {
    RWSTATE_WRITTEN = 0x8000000,
    RWSTATE_COUNTER = 0x7ffffff
};

struct rwstatemap_page {
    struct rwlockmap_page* lockpg;
    unsigned long          state[RWLOCKMAP_PAGE_NENTRIES];
};

static void
rwstatemap_page_init(struct rwstatemap_page* self)
{
    assert(self);

    self->lockpg = NULL;
    memset(self->state, 0, sizeof(self->state));
}

static void
rwstatemap_page_uninit(struct rwstatemap_page* self)
{
    assert(self);
}

static struct rwlockmap_page*
rwstatemap_page_get_global_page(struct rwstatemap_page* self,
                                unsigned long long offset,
                                struct rwlockmap* rwlockmap,
                                struct picotm_error* error)
{
    assert(self);

    if (self->lockpg) {
        return self->lockpg;
    }

    self->lockpg = rwlockmap_find_page(rwlockmap, offset, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return self->lockpg;
}

static void
rwstatemap_page_unlock_regions(struct rwstatemap_page* self,
                               unsigned long long offset,
                               struct rwlockmap* rwlockmap,
                               struct picotm_error* error)
{
    unsigned long long pgoffset;

    assert(self);

    struct rwlockmap_page* lockpg = rwstatemap_page_get_global_page(self,
                                                                    offset,
                                                                    rwlockmap,
                                                                    error);
    if (picotm_error_is_set(error)) {
        return;
    }

    for (pgoffset = 0; pgoffset < RWLOCKMAP_PAGE_NENTRIES; ++pgoffset) {

        if (!(self->state[pgoffset] & RWSTATE_COUNTER)) {
            continue;
        }

        picotm_rwlock_unlock(lockpg->lock + pgoffset);
        self->state[pgoffset] = 0;
    }
}

/*
 * rwstate map
 */

void
rwstatemap_init(struct rwstatemap* self)
{
    assert(self);

    picotm_treemap_init(&self->super, RWLOCKMAP_PAGE_NBITS);
}

static void
rwstatemap_destroy_page_fn(uintptr_t value, struct picotm_treemap* treemap)
{
    struct rwstatemap_page* statepg = (struct rwstatemap_page*)value;

    rwstatemap_page_uninit(statepg);
    free(statepg);
}

void
rwstatemap_uninit(struct rwstatemap* self)
{
    assert(self);

    picotm_treemap_uninit(&self->super, rwstatemap_destroy_page_fn);
}

static uintptr_t
rwstatemap_create_page_fn(unsigned long long offset,
                          struct picotm_treemap* treemap,
                          struct picotm_error* error)
{
    struct rwstatemap_page* statepg = malloc(sizeof(*statepg));
    if (!statepg) {
        picotm_error_set_errno(error, errno);
        return 0;
    }

    rwstatemap_page_init(statepg);

    return (uintptr_t)statepg;
}

static unsigned long long
key_bits(unsigned long long offset, unsigned long page_nbits)
{
    return offset >> page_nbits;
}

void
rwstatemap_for_each_record_in_range(struct rwstatemap* self,
                                    unsigned long long record_length,
                                    unsigned long long record_offset,
                                    struct rwlockmap* rwlockmap,
                                    void (*call_record)(unsigned long*,
                                                        const unsigned long*,
                                                        struct picotm_rwlock*,
                                                        struct picotm_error*),
                                    struct picotm_error* error)
{
    assert(self);

    while (record_length) {

        uintptr_t value = picotm_treemap_find_value(&self->super,
                                                    key_bits(record_offset, RWLOCKMAP_PAGE_NBITS),
                                                    rwstatemap_create_page_fn,
                                                    error);
        if (picotm_error_is_set(error)) {
            return;
        }
        struct rwstatemap_page* statepg = (struct rwstatemap_page*)value;

        struct rwlockmap_page* lockpg =
            rwstatemap_page_get_global_page(statepg, record_offset, rwlockmap, error);

        if (picotm_error_is_set(error)) {
            return;
        }

        /* compute next slice */

        unsigned long long pgoffset = record_offset % RWLOCKMAP_PAGE_NENTRIES;
        unsigned long long diff = llmin(record_length,
                                        RWLOCKMAP_PAGE_NENTRIES - pgoffset);

        record_length -= diff;
        record_offset += diff;

        /* execute call-back */

        call_record(statepg->state + pgoffset,
                    statepg->state + pgoffset + diff,
                    lockpg->lock + pgoffset, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

static void
rdlock_records(unsigned long* state_beg, const unsigned long* state_end,
               struct picotm_rwlock* lock_beg, struct picotm_error* error)
{
    for (; state_beg < state_end; ++state_beg, ++lock_beg) {

        if ( !((*state_beg) & RWSTATE_COUNTER) ) {

            /* not yet read-locked */

            picotm_rwlock_try_rdlock(lock_beg, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }

        *state_beg = ((*state_beg) & RWSTATE_WRITTEN) |
                    (((*state_beg) & RWSTATE_COUNTER) + 1);
    }
}

bool
rwstatemap_rdlock(struct rwstatemap* self,
                  unsigned long long record_length,
                  unsigned long long record_offset,
                  struct rwlockmap* rwlockmap, struct picotm_error* error)
{
    rwstatemap_for_each_record_in_range(self, record_length, record_offset,
                                        rwlockmap, rdlock_records, error);
    if (picotm_error_is_set(error)) {
        return false;
    }
    return true;
}

static void
wrlock_records(unsigned long* state_beg, const unsigned long* state_end,
               struct picotm_rwlock* lock_beg, struct picotm_error* error)
{
    for (; state_beg < state_end; ++state_beg, ++lock_beg) {

        if ( !((*state_beg) & RWSTATE_WRITTEN) ) {

            /* not yet write-locked */

            bool upgrade = !!((*state_beg) & RWSTATE_COUNTER);

            picotm_rwlock_try_wrlock(lock_beg, upgrade, error);
            if (picotm_error_is_set(error)) {
                return;
            }
        }

        *state_beg = RWSTATE_WRITTEN | (((*state_beg) & RWSTATE_COUNTER) + 1);
    }
}

bool
rwstatemap_wrlock(struct rwstatemap* self,
                  unsigned long long record_length,
                  unsigned long long record_offset,
                  struct rwlockmap* rwlockmap, struct picotm_error* error)
{
    rwstatemap_for_each_record_in_range(self, record_length, record_offset,
                                        rwlockmap, wrlock_records, error);
    if (picotm_error_is_set(error)) {
        return false;
    }
    return true;
}

static void
unlock_records(unsigned long* state_beg, const unsigned long* state_end,
               struct picotm_rwlock* lock_beg, struct picotm_error* error)
{
    for (; state_beg < state_end; ++state_beg, ++lock_beg) {

        assert((*state_beg) & RWSTATE_COUNTER);

        *state_beg = ((*state_beg) & RWSTATE_WRITTEN) |
                    (((*state_beg) & RWSTATE_COUNTER) - 1);

        if ( !((*state_beg) & RWSTATE_COUNTER) ) {

            /* last lock of this transaction */

            picotm_rwlock_unlock(lock_beg);
            *state_beg = 0;
        }
    }
}

void
rwstatemap_unlock(struct rwstatemap* self, unsigned long long record_length,
                                           unsigned long long record_offset,
                                           struct rwlockmap* rwlockmap,
                                           struct picotm_error* error)
{
    rwstatemap_for_each_record_in_range(self, record_length, record_offset,
                                        rwlockmap, unlock_records, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
rwstatemap_for_each_page_unlock_regions(uintptr_t value,
                                        unsigned long long offset,
                                        struct picotm_treemap* treemap,
                                        void* rwlockmap,
                                        struct picotm_error* error)
{
    struct rwstatemap_page* statepg = (struct rwstatemap_page*)value;

    rwstatemap_page_unlock_regions(statepg, offset, rwlockmap, error);
}

void
rwstatemap_unlock_all(struct rwstatemap* self,
                      struct rwlockmap* rwlockmap,
                      struct picotm_error* error)
{
    picotm_treemap_for_each_value(&self->super,
                                  rwlockmap,
                                  rwstatemap_for_each_page_unlock_regions,
                                  error);
}
