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

#include "fifo.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-rwstate.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
fifo_init(struct fifo* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_init(&self->lock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    picotm_ref_init(&self->ref, 0);
    file_id_clear(&self->id);

    init_rwlocks(picotm_arraybeg(self->rwlock),
                 picotm_arrayend(self->rwlock));
}

void
fifo_uninit(struct fifo* self)
{
    uninit_rwlocks(picotm_arraybeg(self->rwlock),
                   picotm_arrayend(self->rwlock));

    pthread_rwlock_destroy(&self->lock);
}

static void
fifo_rdlock(struct fifo* self)
{
    assert(self);

    int err = pthread_rwlock_rdlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
fifo_wrlock(struct fifo* self)
{
    assert(self);

    int err = pthread_rwlock_wrlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
fifo_unlock(struct fifo* self)
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
ref_or_set_up(struct fifo* self, int fildes, struct picotm_error* error)
{
    assert(self);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        /* we got a set-up instance; signal success */
        return;
    }

    file_id_init_from_fildes(&self->id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_file_id_init_from_fildes;
    }

    return;

err_file_id_init_from_fildes:
    fifo_unref(self);
}

void
fifo_ref_or_set_up(struct fifo* self, int fildes, struct picotm_error* error)
{
    fifo_wrlock(self);

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

    fifo_unlock(self);

    return;

err_ref_or_set_up:
    fifo_unlock(self);
}

void
fifo_ref(struct fifo* self)
{
    picotm_ref_up(&self->ref);
}

void
fifo_unref(struct fifo* self)
{
    assert(self);

    fifo_wrlock(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        goto unlock;
    }

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    file_id_clear(&self->id);

unlock:
    fifo_unlock(self);
}

int
fifo_cmp_and_ref_or_set_up(struct fifo* self, const struct file_id* id,
                           int fildes, struct picotm_error* error)
{
    assert(self);

    fifo_wrlock(self);

    int cmp = file_id_cmp(&self->id, id);
    if (cmp) {
        goto unlock; /* ids are not equal; only return */
    }

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

unlock:
    fifo_unlock(self);

    return cmp;

err_ref_or_set_up:
    fifo_unlock(self);
    return cmp;
}

int
fifo_cmp_and_ref(struct fifo* self, const struct file_id* id)
{
    assert(self);

    fifo_rdlock(self);

    int cmp = file_id_cmp(&self->id, id);
    if (!cmp) {
        fifo_ref(self);
    }

    fifo_unlock(self);

    return cmp;
}

void
fifo_try_rdlock_field(struct fifo* self, enum fifo_field field,
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
fifo_try_wrlock_field(struct fifo* self, enum fifo_field field,
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
fifo_unlock_field(struct fifo* self, enum fifo_field field,
                  struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}
