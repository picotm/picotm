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

#include "ofd.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-ptr.h>
#include <picotm/picotm-lib-rwstate.h>
#include <stdlib.h>
#include <unistd.h>

static struct ofd*
ofd_of_picotm_shared_ref16_obj(struct picotm_shared_ref16_obj* ref_obj)
{
    return picotm_containerof(ref_obj, struct ofd, ref_obj);
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
ofd_init(struct ofd* self, struct picotm_error* error)
{
    assert(self);

    picotm_shared_ref16_obj_init(&self->ref_obj, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    ofd_id_init(&self->id);

    init_rwlocks(picotm_arraybeg(self->rwlock),
                 picotm_arrayend(self->rwlock));
}

void
ofd_uninit(struct ofd* self)
{
    uninit_rwlocks(picotm_arraybeg(self->rwlock),
                   picotm_arrayend(self->rwlock));

    ofd_id_uninit(&self->id);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_shared_ref16_obj_uninit(&self->ref_obj, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

/*
 * Referencing
 */

struct ref_obj_data {
    const struct ofd_id* id;
    int fildes;
    bool ne_fildes;
    int cmp;
};

static void
first_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct ofd* self = ofd_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    const struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    ofd_id_set_from_fildes(&self->id, ref_obj_data->fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
ofd_ref_or_set_up(struct ofd* self, int fildes, struct picotm_error* error)
{
    struct ref_obj_data data = {
        NULL,
        fildes,
        false,
        0
    };

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, NULL, first_ref,
                               error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
ofd_ref(struct ofd* self)
{
    assert(self);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_shared_ref16_obj_up(&self->ref_obj, NULL, NULL, NULL, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

static void
final_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct ofd* self = ofd_of_picotm_shared_ref16_obj(ref_obj);

    /* We clear the file on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    ofd_id_clear(&self->id);
}

void
ofd_unref(struct ofd* self)
{
    assert(self);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL, final_ref,
                                 &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

static int
cmp_ofd_id(const struct ofd_id* lhs, const struct ofd_id* rhs,
           bool ne_fildes, struct picotm_error* error)
{
    if (ne_fildes) {
        return ofd_id_cmp_ne_fildes(lhs, rhs, error);
    } else {
        return ofd_id_cmp(lhs, rhs);
    }
}

static bool
cond_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
         struct picotm_error* error)
{
    struct ofd* self = ofd_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    ref_obj_data->cmp = cmp_ofd_id(&self->id,
                                   ref_obj_data->id,
                                   ref_obj_data->ne_fildes,
                                   error);
    return !ref_obj_data->cmp;
}

int
ofd_cmp_and_ref_or_set_up(struct ofd* self, const struct ofd_id* id,
                          int fildes, bool ne_fildes,
                          struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = {
        id,
        fildes,
        ne_fildes,
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
ofd_cmp_and_ref(struct ofd* self, const struct ofd_id* id, bool ne_fildes,
                struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = {
        id,
        -1,
        ne_fildes,
        0
    };

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, NULL, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }

    return data.cmp;
}

/*
 * Locking
 */

void
ofd_try_rdlock_field(struct ofd* self, enum ofd_field field,
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
ofd_try_wrlock_field(struct ofd* self, enum ofd_field field,
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
ofd_unlock_field(struct ofd* self, enum ofd_field field,
                 struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}
