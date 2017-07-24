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

#include "dir.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-rwstate.h>
#include <stdlib.h>

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
dir_init(struct dir* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_init(&self->lock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    picotm_ref_init(&self->ref, 0);
    file_id_clear(&self->id);

    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    init_rwlocks(picotm_arraybeg(self->rwlock),
                 picotm_arrayend(self->rwlock));
}

void
dir_uninit(struct dir* self)
{
    uninit_rwlocks(picotm_arraybeg(self->rwlock),
                   picotm_arrayend(self->rwlock));

    pthread_rwlock_destroy(&self->lock);
}

static void
dir_rdlock(struct dir* self)
{
    assert(self);

    int err = pthread_rwlock_rdlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
dir_wrlock(struct dir* self)
{
    assert(self);

    int err = pthread_rwlock_wrlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
dir_unlock(struct dir* self)
{
    assert(self);

    int err = pthread_rwlock_unlock(&self->lock);
    if (err) {
        abort();
    }
}

enum picotm_libc_cc_mode
dir_get_cc_mode(struct dir* self)
{
    assert(self);

    dir_rdlock(self);
    enum picotm_libc_cc_mode cc_mode = self->cc_mode;
    dir_unlock(self);

    return cc_mode;
}

/*
 * Referencing
 */

/* requires internal writer lock */
static void
ref_or_set_up(struct dir* self, int fildes, struct picotm_error* error)
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

    self->cc_mode =
        picotm_libc_get_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_DIR);

    return;

err_file_id_init_from_fildes:
    dir_unref(self);
}

void
dir_ref_or_set_up(struct dir* self, int fildes,
                     struct picotm_error* error)
{
    dir_wrlock(self);

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

    dir_unlock(self);

    return;

err_ref_or_set_up:
    dir_unlock(self);
}

void
dir_ref(struct dir* self)
{
    picotm_ref_up(&self->ref);
}

void
dir_unref(struct dir* self)
{
    assert(self);

    dir_wrlock(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        goto unlock;
    }

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    file_id_clear(&self->id);

unlock:
    dir_unlock(self);
}

int
dir_cmp_and_ref_or_set_up(struct dir* self, const struct file_id* id,
                          int fildes, struct picotm_error* error)
{
    assert(self);

    dir_wrlock(self);

    int cmp = file_id_cmp(&self->id, id);
    if (cmp) {
        goto unlock; /* ids are not equal; only return */
    }

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

unlock:
    dir_unlock(self);

    return cmp;

err_ref_or_set_up:
    dir_unlock(self);
    return cmp;
}

int
dir_cmp_and_ref(struct dir* self, const struct file_id* id)
{
    assert(self);

    dir_rdlock(self);

    int cmp = file_id_cmp(&self->id, id);
    if (!cmp) {
        dir_ref(self);
    }

    dir_unlock(self);

    return cmp;
}

void
dir_try_rdlock_field(struct dir* self, enum dir_field field,
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
dir_try_wrlock_field(struct dir* self, enum dir_field field,
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
dir_unlock_field(struct dir* self, enum dir_field field,
                 struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}
