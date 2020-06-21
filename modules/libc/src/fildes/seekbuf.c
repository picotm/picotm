/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "seekbuf.h"
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
seekbuf_init(struct seekbuf* self, struct picotm_error* error)
{
    assert(self);

    if (picotm_error_is_set(error)) {
        return;
    }

    init_rwlocks(picotm_arraybeg(self->rwlock),
                 picotm_arrayend(self->rwlock));

    rwlockmap_init(&self->rwlockmap);
}

void
seekbuf_uninit(struct seekbuf* self)
{
    rwlockmap_uninit(&self->rwlockmap);

    uninit_rwlocks(picotm_arraybeg(self->rwlock),
                   picotm_arrayend(self->rwlock));
}

/*
 * Reference counting
 */

static struct seekbuf*
seekbuf_of_ref_obj(struct picotm_shared_ref16_obj* ref_obj)
{
    return picotm_containerof(ref_obj, struct seekbuf, ref_obj);
}

struct ref_obj_data {
    const struct filebuf_id* id;
    int fildes;
    int cmp;
};

#define REF_OBJ_DATA_INITIALIZER(id_, fildes_, cmp_)    \
    {                                                   \
        .id = (id_),                                    \
        .fildes = (fildes_),                            \
        .cmp = (cmp_),                                  \
    }

static void
first_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct seekbuf* self = seekbuf_of_ref_obj(ref_obj);
    assert(self);

    const struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    filebuf_id_init_from_fildes(&self->id, ref_obj_data->fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
seekbuf_ref_or_set_up(struct seekbuf* self, int fildes, struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(NULL, fildes, 0);
    picotm_shared_ref16_obj_up(&self->ref_obj, &data, NULL, first_ref,
                               error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static bool
cond_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
         struct picotm_error* error)
{
    struct seekbuf* self = seekbuf_of_ref_obj(ref_obj);
    assert(self);

    struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    ref_obj_data->cmp = filebuf_id_cmp(&self->id, ref_obj_data->id);

    return !ref_obj_data->cmp;
}

int
seekbuf_ref_or_set_up_if_id(struct seekbuf* self, int fildes,
                            const struct filebuf_id* id,
                            struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(id, fildes, 0);
    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, first_ref,
                               error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return data.cmp;
}

int
seekbuf_ref_if_id(struct seekbuf* self, const struct filebuf_id* id)
{
    assert(self);

    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(id, -1, 0);
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;
    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, NULL, &error);
    if (picotm_error_is_set(&error)) {
        return 0;
    }

    return data.cmp;
}

void
seekbuf_ref(struct seekbuf* self, struct picotm_error* error)
{
    assert(self);

    picotm_shared_ref16_obj_up(&self->ref_obj, NULL, NULL, NULL, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
final_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct seekbuf* self = seekbuf_of_ref_obj(ref_obj);
    assert(self);

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    filebuf_id_clear(&self->id);
}

void
seekbuf_unref(struct seekbuf* self)
{
    assert(self);

    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL, final_ref);
}

void
seekbuf_try_rdlock_field(struct seekbuf* self, enum seekbuf_field field,
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
seekbuf_try_wrlock_field(struct seekbuf* self, enum seekbuf_field field,
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
seekbuf_unlock_field(struct seekbuf* self, enum seekbuf_field field,
                     struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}

void
seekbuf_try_rdlock_region(struct seekbuf* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error)
{
    assert(self);

    rwcountermap_rdlock(rwcountermap, reccount(nbyte), recoffset(off),
                        &self->rwlockmap, error);
}

void
seekbuf_try_wrlock_region(struct seekbuf* self, off_t off, size_t nbyte,
                          struct rwcountermap* rwcountermap,
                          struct picotm_error* error)
{
    assert(self);

    rwcountermap_wrlock(rwcountermap, reccount(nbyte), recoffset(off),
                        &self->rwlockmap, error);
}

void
seekbuf_unlock_region(struct seekbuf* self, off_t off, size_t nbyte,
                      struct rwcountermap* rwcountermap)
{
    assert(self);

    rwcountermap_unlock(rwcountermap,
                        reccount(nbyte),
                        recoffset(off),
                        &self->rwlockmap);
}