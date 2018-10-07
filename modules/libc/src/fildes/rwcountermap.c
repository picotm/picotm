/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "rwcountermap.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "range.h"
#include "rwcounter.h"
#include "rwlockmap.h"

/*
 * rwstate_page
 */

struct rwcountermap_page {
    struct rwlockmap_page* lockpg;
    struct rwcounter       counter[RWLOCKMAP_PAGE_NENTRIES];
};

static void
rwcountermap_page_init(struct rwcountermap_page* self)
{
    assert(self);

    self->lockpg = NULL;

    struct rwcounter* counter_beg = picotm_arraybeg(self->counter);
    struct rwcounter* counter_end = picotm_arrayend(self->counter);

    while (counter_beg < counter_end) {
        rwcounter_init(counter_beg);
        ++counter_beg;
    }
}

static void
rwcountermap_page_uninit(struct rwcountermap_page* self)
{
    assert(self);
}

static struct rwlockmap_page*
rwcountermap_page_get_rwlockmap_page(struct rwcountermap_page* self,
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

static struct rwcounter*
call_records(struct rwcounter* counter_beg,
             const struct rwcounter* counter_end,
             struct picotm_rwlock* lock_beg,
             void (*call_record)(struct rwcounter*, struct picotm_rwlock*,
                                 struct picotm_error*),
             struct picotm_error* error)
{
    for (; counter_beg < counter_end; ++counter_beg, ++lock_beg) {

        call_record(counter_beg, lock_beg, error);
        if (picotm_error_is_set(error)) {
            return counter_beg;
        }
    }

    return counter_beg;
}

static unsigned long long
rwcountermap_page_for_each_record_in_range(
    struct rwcountermap_page* self,
    unsigned long long record_length, unsigned long long record_offset,
    struct rwlockmap* rwlockmap, void (*call_record)(struct rwcounter*,
                                                     struct picotm_rwlock*,
                                                     struct picotm_error*),
    struct picotm_error* error)
{
    struct rwlockmap_page* lockpg =
        rwcountermap_page_get_rwlockmap_page(self, record_offset,
                                             rwlockmap, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    /* compute record range within page */

    unsigned long long offset = record_offset % RWLOCKMAP_PAGE_NENTRIES;
    unsigned long long length = llmin(record_length,
                                      RWLOCKMAP_PAGE_NENTRIES - offset);

    /* execute call-back */

    const struct rwcounter* end = call_records(self->counter + offset,
                                               self->counter + offset + length,
                                               lockpg->lock + offset,
                                               call_record, error);
    if (picotm_error_is_set(error)) {
        return end - (self->counter + offset);
    }

    return end - (self->counter + offset);
}

/*
 * rwstate map
 */

void
rwcountermap_init(struct rwcountermap* self)
{
    assert(self);

    picotm_treemap_init(&self->map, RWLOCKMAP_PAGE_NBITS);
}

static void
rwcountermap_destroy_page_fn(uintptr_t value, struct picotm_treemap* treemap)
{
    struct rwcountermap_page* statepg = (struct rwcountermap_page*)value;

    rwcountermap_page_uninit(statepg);
    free(statepg);
}

void
rwcountermap_uninit(struct rwcountermap* self)
{
    assert(self);

    picotm_treemap_uninit(&self->map, rwcountermap_destroy_page_fn);
}

static uintptr_t
rwcountermap_create_page_fn(unsigned long long offset,
                            struct picotm_treemap* treemap,
                            struct picotm_error* error)
{
    struct rwcountermap_page* statepg = malloc(sizeof(*statepg));
    if (!statepg) {
        picotm_error_set_errno(error, errno);
        return 0;
    }

    rwcountermap_page_init(statepg);

    return (uintptr_t)statepg;
}

static unsigned long long
key_bits(unsigned long long offset, unsigned long page_nbits)
{
    return offset >> page_nbits;
}

void
rwcountermap_for_each_record_in_range(
    struct rwcountermap* self,
    unsigned long long record_length, unsigned long long record_offset,
    struct rwlockmap* rwlockmap, void (*call_record)(struct rwcounter*,
                                                     struct picotm_rwlock*,
                                                     struct picotm_error*),
    struct picotm_error* error)
{
    assert(self);

    while (record_length) {

        uintptr_t value = picotm_treemap_find_value(
            &self->map, key_bits(record_offset, RWLOCKMAP_PAGE_NBITS),
            rwcountermap_create_page_fn, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        struct rwcountermap_page* statepg = (struct rwcountermap_page*)value;

        unsigned long long diff =
            rwcountermap_page_for_each_record_in_range(statepg,
                                                       record_length,
                                                       record_offset,
                                                       rwlockmap,
                                                       call_record,
                                                       error);
        if (picotm_error_is_set(error)) {
            return;
        }

        record_length -= diff;
        record_offset += diff;
    }
}

static void
rdlock_record(struct rwcounter* counter, struct picotm_rwlock* lock,
              struct picotm_error* error)
{
    rwcounter_rdlock(counter, lock, error);
}

void
rwcountermap_rdlock(struct rwcountermap* self,
                  unsigned long long record_length,
                  unsigned long long record_offset,
                  struct rwlockmap* rwlockmap, struct picotm_error* error)
{
    rwcountermap_for_each_record_in_range(self, record_length, record_offset,
                                          rwlockmap, rdlock_record, error);
}

static void
wrlock_record(struct rwcounter* counter, struct picotm_rwlock* lock,
              struct picotm_error* error)
{
    rwcounter_wrlock(counter, lock, error);
}

void
rwcountermap_wrlock(struct rwcountermap* self,
                    unsigned long long record_length,
                    unsigned long long record_offset,
                    struct rwlockmap* rwlockmap, struct picotm_error* error)
{
    rwcountermap_for_each_record_in_range(self, record_length, record_offset,
                                          rwlockmap, wrlock_record, error);
}

static void
unlock_record(struct rwcounter* counter, struct picotm_rwlock* lock,
              struct picotm_error* error)
{
    rwcounter_unlock(counter, lock);
}

void
rwcountermap_unlock(struct rwcountermap* self,
                    unsigned long long record_length,
                    unsigned long long record_offset,
                    struct rwlockmap* rwlockmap)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        rwcountermap_for_each_record_in_range(self, record_length,
                                              record_offset, rwlockmap,
                                              unlock_record, &error);
        if (picotm_error_is_set(&error)) {
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}

static void
rwcountermap_for_each_page_unlock_regions(uintptr_t value,
                                          unsigned long long offset,
                                          struct picotm_treemap* treemap,
                                          void* rwlockmap,
                                          struct picotm_error* error)
{
    struct rwcountermap* statemap =
        picotm_containerof(treemap, struct rwcountermap, map);

    rwcountermap_for_each_record_in_range(statemap, RWLOCKMAP_PAGE_NENTRIES,
                                          offset, rwlockmap, unlock_record,
                                          error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
rwcountermap_unlock_all(struct rwcountermap* self,
                        struct rwlockmap* rwlockmap,
                        struct picotm_error* error)
{
    picotm_treemap_for_each_value(&self->map,
                                  rwlockmap,
                                  rwcountermap_for_each_page_unlock_regions,
                                  error);
}
