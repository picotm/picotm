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

#include "regfile.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-rwstate.h"
#include "rwcountermap.h"

#define RECSIZE (1ul << RECBITS)

static off_t
recoffset(off_t off)
{
    return off/RECSIZE;
}

static size_t
reccount(size_t len)
{
    return 1 + len/RECSIZE;
}

static void
init_rwlocks(struct picotm_rwlock* beg, const struct picotm_rwlock* end)
{
    while (beg < end) {
        picotm_rwlock_init(beg);
        ++beg;
    }
}

static void
uninit_rwlocks(struct picotm_rwlock* beg, const struct picotm_rwlock* end)
{
    while (beg < end) {
        picotm_rwlock_uninit(beg);
        ++beg;
    }
}

void
regfile_init(struct regfile* self, struct picotm_error* error)
{
    assert(self);

    file_init(&self->base, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    init_rwlocks(picotm_arraybeg(self->rwlock),
                 picotm_arrayend(self->rwlock));

    rwlockmap_init(&self->rwlockmap);
}

void
regfile_uninit(struct regfile* self)
{
    rwlockmap_uninit(&self->rwlockmap);

    uninit_rwlocks(picotm_arraybeg(self->rwlock),
                   picotm_arrayend(self->rwlock));

    file_uninit(&self->base);
}

void
regfile_try_rdlock_field(struct regfile* self, enum regfile_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(rwstate, self->rwlock + field, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
regfile_try_wrlock_field(struct regfile* self, enum regfile_field field,
                         struct picotm_rwstate* rwstate,
                         struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_wrlock(rwstate, self->rwlock + field, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
regfile_unlock_field(struct regfile* self, enum regfile_field field,
                     struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}

void
regfile_try_rdlock_region(struct regfile* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error)
{
    assert(self);

    rwcountermap_rdlock(rwcountermap, reccount(nbyte), recoffset(off),
                        &self->rwlockmap, error);
}

void
regfile_try_wrlock_region(struct regfile* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error)
{
    assert(self);

    rwcountermap_wrlock(rwcountermap, reccount(nbyte), recoffset(off),
                        &self->rwlockmap, error);
}

void
regfile_unlock_region(struct regfile* self, off_t off, size_t nbyte,
                      struct rwcountermap* rwcountermap)
{
    assert(self);

    rwcountermap_unlock(rwcountermap,
                        reccount(nbyte),
                        recoffset(off),
                        &self->rwlockmap);
}
