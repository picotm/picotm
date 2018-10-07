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

#include "socket.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-rwstate.h"
#include <assert.h>
#include <stdlib.h>

static struct socket*
socket_of_picotm_shared_ref16_obj(struct picotm_shared_ref16_obj* ref_obj)
{
    return picotm_containerof(ref_obj, struct socket, ref_obj);
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
socket_init(struct socket* self, struct picotm_error* error)
{
    assert(self);

    picotm_shared_ref16_obj_init(&self->ref_obj, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    file_id_clear(&self->id);

    init_rwlocks(picotm_arraybeg(self->rwlock),
                 picotm_arrayend(self->rwlock));
}

void
socket_uninit(struct socket* self)
{
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
    struct socket* self = socket_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    const struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    file_id_init_from_fildes(&self->id, ref_obj_data->fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
socket_ref_or_set_up(struct socket* self, int fildes,
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
socket_ref(struct socket* self, struct picotm_error* error)
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
    struct socket* self = socket_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    file_id_clear(&self->id);
}

void
socket_unref(struct socket* self)
{
    assert(self);

    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL, final_ref);
}

static bool
cond_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
         struct picotm_error* error)
{
    struct socket* self = socket_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    ref_obj_data->cmp = file_id_cmp(&self->id, ref_obj_data->id);

    return !ref_obj_data->cmp;
}

int
socket_cmp_and_ref_or_set_up(struct socket* self, const struct file_id* id,
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
socket_cmp_and_ref(struct socket* self, const struct file_id* id)
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
socket_try_rdlock_field(struct socket* self, enum socket_field field,
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
socket_try_wrlock_field(struct socket* self, enum socket_field field,
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
socket_unlock_field(struct socket* self, enum socket_field field,
                    struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}
