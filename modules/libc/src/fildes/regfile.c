/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
 */

#include "regfile.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-rwstate.h"
#include <assert.h>
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

static struct regfile*
regfile_of_picotm_shared_ref16_obj(struct picotm_shared_ref16_obj* ref_obj)
{
    return picotm_containerof(ref_obj, struct regfile, ref_obj);
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

    picotm_shared_ref16_obj_init(&self->ref_obj, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    file_id_clear(&self->id);

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

    picotm_shared_ref16_obj_uninit(&self->ref_obj);
}

/*
 * Referencing
 */

struct ref_obj_data {
    const struct file_id* id;
    int fildes;
    int cmp;
};

static void
first_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct regfile* self = regfile_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    const struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    file_id_init_from_fildes(&self->id, ref_obj_data->fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
regfile_ref_or_set_up(struct regfile* self, int fildes,
                      struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = {
        NULL,
        fildes,
        0
    };

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, NULL, first_ref,
                               error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
regfile_ref(struct regfile* self, struct picotm_error* error)
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
    struct regfile* self = regfile_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    file_id_clear(&self->id);
}

void
regfile_unref(struct regfile* self)
{
    assert(self);

    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL, final_ref);
}

static bool
cond_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
         struct picotm_error* error)
{
    struct regfile* self = regfile_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    ref_obj_data->cmp = file_id_cmp(&self->id, ref_obj_data->id);

    return !ref_obj_data->cmp;
}

int
regfile_cmp_and_ref_or_set_up(struct regfile* self, const struct file_id* id,
                              int fildes, struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = {
        id,
        fildes,
        0
    };

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, first_ref,
                               error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return data.cmp;
}

int
regfile_cmp_and_ref(struct regfile* self, const struct file_id* id)
{
    assert(self);

    struct ref_obj_data data = {
        id,
        -1,
        0
    };

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, NULL, &error);
    if (picotm_error_is_set(&error)) {
        return 0;
    }

    return data.cmp;
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
