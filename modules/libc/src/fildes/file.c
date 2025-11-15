/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
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

#include "file.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-ptr.h"
#include <assert.h>

static struct file*
file_of_picotm_shared_ref16_obj(struct picotm_shared_ref16_obj* ref_obj)
{
    return picotm_containerof(ref_obj, struct file, ref_obj);
}

void
file_init(struct file* self, struct picotm_error* error)
{
    assert(self);

    picotm_shared_ref16_obj_init(&self->ref_obj, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    file_id_clear(&self->id);
}

void
file_uninit(struct file* self)
{
    assert(self);

    picotm_shared_ref16_obj_uninit(&self->ref_obj);
}

/*
 * Reference counting
 */

struct ref_obj_data {
    const struct file_id* id;
    int fildes;
    bool new_file;
    int cmp;
};

#define REF_OBJ_DATA_INITIALIZER(id_, fildes_, new_file_, cmp_) \
    {                                                           \
        .id = (id_),                                            \
        .fildes = (fildes_),                                    \
        .new_file = (new_file_),                                \
        .cmp = (cmp_),                                          \
    }

static void
first_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct file* self = file_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    const struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    file_id_init_from_fildes(&self->id, ref_obj_data->fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
file_ref_or_set_up(struct file* self, int fildes, struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(NULL, fildes,
                                                        false, 0);
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
    struct file* self = file_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    const struct file_id* lhs = &self->id;
    const struct file_id* rhs = ref_obj_data->id;

    if (ref_obj_data->new_file)
        ref_obj_data->cmp = file_id_cmp_eq_fildes(lhs, rhs, error);
    else
        ref_obj_data->cmp = file_id_cmp(lhs, rhs);

    return !ref_obj_data->cmp;
}

int
file_ref_or_set_up_if_id(struct file* self, int fildes, bool new_file,
                         const struct file_id* id,
                         struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(id, fildes,
                                                        new_file, 0);
    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, first_ref,
                               error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return data.cmp;
}

int
file_ref_if_id(struct file* self, const struct file_id* id, bool new_file,
               struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(id, -1, new_file, 0);

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, NULL, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return data.cmp;
}

void
file_ref(struct file* self, struct picotm_error* error)
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
    struct file* self = file_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    file_id_clear(&self->id);
}

void
file_unref(struct file* self)
{
    assert(self);

    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL, final_ref);
}
