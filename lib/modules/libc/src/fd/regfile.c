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

#include "regfile.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-rwstate.h>
#include <stdlib.h>
#include <unistd.h>
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

    self->offset = 0;
    rwlockmap_init(&self->rwlockmap);
}

void
regfile_uninit(struct regfile* self)
{
    rwlockmap_uninit(&self->rwlockmap);

    uninit_rwlocks(picotm_arraybeg(self->rwlock),
                   picotm_arrayend(self->rwlock));

    pthread_rwlock_destroy(&self->lock);
}

static void
regfile_rdlock(struct regfile* self)
{
    assert(self);

    int err = pthread_rwlock_rdlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
regfile_wrlock(struct regfile* self)
{
    assert(self);

    int err = pthread_rwlock_wrlock(&self->lock);
    if (err) {
        abort();
    }
}

static void
regfile_unlock(struct regfile* self)
{
    assert(self);

    int err = pthread_rwlock_unlock(&self->lock);
    if (err) {
        abort();
    }
}

enum picotm_libc_cc_mode
regfile_get_cc_mode(struct regfile* self)
{
    assert(self);

    regfile_rdlock(self);
    enum picotm_libc_cc_mode cc_mode = self->cc_mode;
    regfile_unlock(self);

    return cc_mode;
}

off_t
regfile_get_offset(struct regfile* self)
{
    assert(self);

    regfile_rdlock(self);
    off_t offset = self->offset;
    regfile_unlock(self);

    return offset;
}

/*
 * Referencing
 */

/* requires internal writer lock */
static void
ref_or_set_up(struct regfile* self, int fildes, struct picotm_error* error)
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
        picotm_libc_get_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR);

    off_t res = lseek(fildes, 0, SEEK_CUR);
    if (res == (off_t)-1) {
        picotm_error_set_errno(error, errno);
        goto err_lseek;
    }
    self->offset = res;

    return;

err_lseek:
err_file_id_init_from_fildes:
    regfile_unref(self);
}

void
regfile_ref_or_set_up(struct regfile* self, int fildes,
                      struct picotm_error* error)
{
    regfile_wrlock(self);

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

    regfile_unlock(self);

    return;

err_ref_or_set_up:
    regfile_unlock(self);
}

void
regfile_ref(struct regfile* self)
{
    picotm_ref_up(&self->ref);
}

void
regfile_unref(struct regfile* self)
{
    assert(self);

    regfile_wrlock(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        goto unlock;
    }

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    file_id_clear(&self->id);

unlock:
    regfile_unlock(self);
}

int
regfile_cmp_and_ref_or_set_up(struct regfile* self, const struct file_id* id,
                              int fildes, struct picotm_error* error)
{
    assert(self);

    regfile_wrlock(self);

    int cmp = file_id_cmp(&self->id, id);
    if (cmp) {
        goto unlock; /* ids are not equal; only return */
    }

    ref_or_set_up(self, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_ref_or_set_up;
    }

unlock:
    regfile_unlock(self);

    return cmp;

err_ref_or_set_up:
    regfile_unlock(self);
    return cmp;
}

int
regfile_cmp_and_ref(struct regfile* self, const struct file_id* id)
{
    assert(self);

    regfile_rdlock(self);

    int cmp = file_id_cmp(&self->id, id);
    if (!cmp) {
        regfile_ref(self);
    }

    regfile_unlock(self);

    return cmp;
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

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    rwcountermap_unlock(rwcountermap,
                        reccount(nbyte),
                        recoffset(off),
                        &self->rwlockmap,
                        &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}
