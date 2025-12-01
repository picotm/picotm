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

#include "filebuf.h"
#include "picotm/picotm-lib-ptr.h"

void
filebuf_init(struct filebuf self[static 1],
             struct picotm_error error[static 1])
{
    picotm_shared_ref16_obj_init(&self->ref_obj, error);
    if (picotm_error_is_set(error))
        return;

    filebuf_id_clear(&self->id);
}

void
filebuf_uninit(struct filebuf self[static 1])
{
    picotm_shared_ref16_obj_uninit(&self->ref_obj);
}

static struct filebuf*
filebuf_of_ref_obj(struct picotm_shared_ref16_obj ref_obj[static 1])
{
    return picotm_containerof(ref_obj, struct filebuf, ref_obj);
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
first_ref(struct picotm_shared_ref16_obj ref_obj[static 1], void* data,
          struct picotm_error error[static 1])
{
    struct filebuf* self = filebuf_of_ref_obj(ref_obj);
    assert(self);

    const struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    filebuf_id_init_from_fildes(&self->id, ref_obj_data->fildes, error);
    if (picotm_error_is_set(error))
        return;
}

void
filebuf_ref_or_set_up(struct filebuf self[static 1], int fildes,
                      struct picotm_error error[static 1])
{
    assert(self);

    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(nullptr, fildes, 0);
    picotm_shared_ref16_obj_up(&self->ref_obj, &data, nullptr, first_ref,
                               error);
    if (picotm_error_is_set(error))
        return;
}

static bool
cond_ref(struct picotm_shared_ref16_obj ref_obj[static 1], void* data,
         struct picotm_error error[static 1])
{
    struct filebuf* self = filebuf_of_ref_obj(ref_obj);
    assert(self);

    struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    ref_obj_data->cmp = filebuf_id_cmp(&self->id, ref_obj_data->id);

    return !ref_obj_data->cmp;
}

int
filebuf_ref_or_set_up_if_id(struct filebuf self[static 1], int fildes,
                            const struct filebuf_id id[static 1],
                            struct picotm_error error[static 1])
{
    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(id, fildes, 0);

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, first_ref,
                               error);
    if (picotm_error_is_set(error))
        return 0;

    return data.cmp;
}

int
filebuf_ref_if_id(struct filebuf self[static 1],
                  const struct filebuf_id id[static 1])
{
    struct ref_obj_data data = REF_OBJ_DATA_INITIALIZER(id, -1, 0);
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, nullptr, &error);
    if (picotm_error_is_set(&error))
        return 0;

    return data.cmp;
}

void
filebuf_ref(struct filebuf self[static 1],
            struct picotm_error error[static 1])
{
    picotm_shared_ref16_obj_up(&self->ref_obj, nullptr, nullptr, nullptr, error);
    if (picotm_error_is_set(error))
        return;
}

static void
final_ref(struct picotm_shared_ref16_obj ref_obj[static 1], void* data,
          struct picotm_error error[static 1])
{
    struct filebuf* self = filebuf_of_ref_obj(ref_obj);
    assert(self);

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use.
     */
    filebuf_id_clear(&self->id);
}

void
filebuf_unref(struct filebuf self[static 1])
{
    picotm_shared_ref16_obj_down(&self->ref_obj, nullptr, nullptr, final_ref);
}
