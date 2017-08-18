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
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-rwstate.h>
#include <stdlib.h>
#include <unistd.h>

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

    int err = pthread_rwlock_init(&self->lock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    picotm_ref_init(&self->ref, 0);
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

    pthread_rwlock_destroy(&self->lock);
}

static void
ofd_rdlock(struct ofd* self)
{
    assert(self);

    int err = pthread_rwlock_rdlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
ofd_wrlock(struct ofd* self)
{
    assert(self);

    int err = pthread_rwlock_wrlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
ofd_unlock(struct ofd* self)
{
    assert(self);

    int err = pthread_rwlock_unlock(&self->lock);
    if (err) {
        abort();
    }
}

/*
 * Referencing
 */

/* requires internal writer lock */
static void
ref_or_set_up(struct ofd* self, int fildes, struct picotm_error* error)
{
    assert(self);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        /* we got a set-up instance; signal success */
        return;
    }

    ofd_id_set_from_fildes(&self->id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ofdid_init_from_fildes;
    }

    return;

err_ofdid_init_from_fildes:
    ofd_unref(self);
}

void
ofd_ref_or_set_up(struct ofd* self, int fildes, struct picotm_error* error)
{
    ofd_wrlock(self);

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

    ofd_unlock(self);

    return;

err_ref_or_set_up:
    ofd_unlock(self);
}

void
ofd_ref(struct ofd* self)
{
    picotm_ref_up(&self->ref);
}

void
ofd_unref(struct ofd* self)
{
    assert(self);

    ofd_wrlock(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        goto unlock;
    }

    /* We clear the file on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    ofd_id_clear(&self->id);

unlock:
    ofd_unlock(self);
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

int
ofd_cmp_and_ref_or_set_up(struct ofd* self, const struct ofd_id* id,
                          int fildes, bool ne_fildes,
                          struct picotm_error* error)
{
    assert(self);

    ofd_wrlock(self);

    int cmp = cmp_ofd_id(&self->id, id, ne_fildes, error);
    if (cmp) {
        goto unlock; /* ids are not equal; only return */
    }

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

unlock:
    ofd_unlock(self);

    return cmp;

err_ref_or_set_up:
    ofd_unlock(self);
    return cmp;
}

int
ofd_cmp_and_ref(struct ofd* self, const struct ofd_id* id, bool ne_fildes,
                struct picotm_error* error)
{
    assert(self);

    ofd_rdlock(self);

    int cmp = cmp_ofd_id(&self->id, id, ne_fildes, error);
    if (!cmp) {
        ofd_ref(self);
    }

    ofd_unlock(self);

    return cmp;
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
