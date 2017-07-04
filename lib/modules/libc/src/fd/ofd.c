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
#include <limits.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-rwstate.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rwcountermap.h"

#define RECSIZE (1ul << RECBITS)

static void
ofd_set_type(struct ofd* self, enum picotm_libc_file_type type)
{
    assert(self);

    self->type = type;
}

static void
ofd_set_ccmode(struct ofd* self, enum picotm_libc_cc_mode cc_mode)
{
    assert(self);

    self->cc_mode = cc_mode;
}

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

void
ofd_init(struct ofd* self, struct picotm_error* error)
{
    assert(self);

    pthread_rwlockattr_t rwlockattr;
    int err = pthread_rwlockattr_init(&rwlockattr);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    err = pthread_rwlockattr_setkind_np(&rwlockattr,
                                        PTHREAD_RWLOCK_PREFER_WRITER_NP);
    if (err) {
        picotm_error_set_errno(error, err);
        goto err_pthread_rwlockattr_setkind_np;
    }

    err = pthread_rwlock_init(&self->lock, &rwlockattr);
    if (err) {
        picotm_error_set_errno(error, err);
        goto err_pthread_rwlock_init;
    }

    ofdid_clear(&self->id);

    picotm_ref_init(&self->ref, 0);

    self->flags = 0;
    self->type = PICOTM_LIBC_FILE_TYPE_OTHER;

    picotm_rwlock_init(&self->rwlock);

    self->cc_mode = PICOTM_LIBC_CC_MODE_NOUNDO;

    /* Type specific data */

    self->data.regular.offset = 0;

    rwlockmap_init(&self->data.regular.rwlockmap);

    pthread_rwlockattr_destroy(&rwlockattr);

    return;

err_pthread_rwlock_init:
err_pthread_rwlockattr_setkind_np:
    pthread_rwlockattr_destroy(&rwlockattr);
}

void
ofd_uninit(struct ofd* self)
{
    rwlockmap_uninit(&self->data.regular.rwlockmap);

    picotm_rwlock_uninit(&self->rwlock);

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

enum picotm_libc_cc_mode
ofd_get_ccmode_nolock(const struct ofd* self)
{
    return self->cc_mode;
}

enum picotm_libc_file_type
ofd_get_type_nolock(const struct ofd* self)
{
	return self->type;
}

off_t
ofd_get_offset_nolock(const struct ofd* self)
{
    assert(self);

    off_t pos;

    switch (ofd_get_type_nolock(self)) {
        case PICOTM_LIBC_FILE_TYPE_REGULAR:
            pos = self->data.regular.offset;
            break;
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            pos = 0;
            break;
        default:
            abort();
    }

    return pos;
}

/*
 * Referencing
 */

static void
ofd_setup_from_fildes(struct ofd* self, int fildes, bool unlink_file,
                      struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);

    /* Any setup code goes here */

    ofdid_init_from_fildes(&self->id, fildes, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    /* Set type */

	struct stat buf;
    int res = fstat(fildes, &buf);
	if (res < 0) {
        picotm_error_set_errno(error, errno);
		return;
	}

    if (S_ISREG(buf.st_mode)) {
        ofd_set_type(self, PICOTM_LIBC_FILE_TYPE_REGULAR);
    } else if (S_ISFIFO(buf.st_mode)){
        ofd_set_type(self, PICOTM_LIBC_FILE_TYPE_FIFO);
    } else if (S_ISSOCK(buf.st_mode)){
        ofd_set_type(self, PICOTM_LIBC_FILE_TYPE_SOCKET);
    } else {
        ofd_set_type(self, PICOTM_LIBC_FILE_TYPE_OTHER);
    }

    ofd_set_ccmode(self, picotm_libc_get_file_type_cc_mode(ofd_get_type_nolock(self)));

    switch (ofd_get_type_nolock(self)) {
        case PICOTM_LIBC_FILE_TYPE_REGULAR: {
            off_t res = lseek(fildes, 0, SEEK_CUR);
            if (res == (off_t)-1) {
                picotm_error_set_errno(error, errno);
                return;
            }
            self->data.regular.offset = res;
            break;
        }
        case PICOTM_LIBC_FILE_TYPE_FIFO:
        case PICOTM_LIBC_FILE_TYPE_OTHER:
        case PICOTM_LIBC_FILE_TYPE_SOCKET:
            break;
        default:
            abort();
    }

    /* Set flags */
    self->flags = 0;
    if (unlink_file) {
        self->flags |= OFD_FL_UNLINK;
    }
}

void
ofd_ref_or_set_up(struct ofd* self, int fildes, bool unlink_file,
                  struct picotm_error* error)
{
    assert(self);
    assert(fildes >= 0);

    ofd_wrlock(self);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        /* we got a set-up instance; signal success */
        goto unlock;
    }

    /* Setup ofd for given file descriptor */
    ofd_setup_from_fildes(self, fildes, unlink_file, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_setup_from_fildes;
    }

unlock:
    ofd_unlock(self);

    return;

err_ofd_setup_from_fildes:
    ofd_unref(self);
    ofd_unlock(self);
}

void
ofd_ref(struct ofd* self)
{
    picotm_ref_up(&self->ref);
}

void
ofd_ref_state(struct ofd* self,
              enum picotm_libc_file_type* type,
              enum picotm_libc_cc_mode* ccmode,
              off_t* offset)
{
    ofd_ref(self);

    ofd_rdlock(self);

    if (type) {
        *type = ofd_get_type_nolock(self);
    }
    if (ccmode) {
        *ccmode = ofd_get_ccmode_nolock(self);
    }
    if (offset) {
        *offset = ofd_get_offset_nolock(self);
    }

    ofd_unlock(self);
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

    /* We clear the id on releasing the final reference. This
     * instance remains initialized, but is available for later
     * use. */
    ofdid_clear(&self->id);

unlock:
    ofd_unlock(self);
}

int
ofd_cmp_and_ref_or_set_up(struct ofd* self, const struct ofdid* id,
                          int fildes, bool unlink_file,
                          struct picotm_error* error)
{
    assert(self);

    ofd_wrlock(self);

    int cmp = ofdidcmp(&self->id, id);
    if (cmp) {
        goto unlock; /* ids are not equal; only return */
    }

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        /* we got a set-up instance; signal success */
        goto unlock;
    }

    /* Setup ofd for given file descriptor */
    ofd_setup_from_fildes(self, fildes, unlink_file, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_setup_from_fildes;
    }

unlock:
    ofd_unlock(self);

    return cmp;

err_ofd_setup_from_fildes:
    ofd_unref(self);
    ofd_unlock(self);
    return cmp;
}

int
ofd_cmp_and_ref(struct ofd* self, const struct ofdid* id)
{
    assert(self);

    ofd_rdlock(self);

    int cmp = ofdidcmp(&self->id, id);
    if (!cmp) {
        ofd_ref(self);
    }

    ofd_unlock(self);

    return cmp;
}

void
ofd_rdlock_state(struct ofd* self, struct picotm_rwstate* rwstate,
                 struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_rdlock(rwstate, &self->rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
ofd_wrlock_state(struct ofd* self, struct picotm_rwstate* rwstate,
                 struct picotm_error* error)
{
    assert(self);

    picotm_rwstate_try_wrlock(rwstate, &self->rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
ofd_unlock_state(struct ofd* self, struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, &self->rwlock);
}

void
ofd_2pl_lock_region(struct ofd* self, off_t off, size_t nbyte, bool write,
                    struct rwcountermap* rwcountermap,
                    struct picotm_error* error)
{
    static void (* const lock[])(struct rwcountermap*,
                                 unsigned long long,
                                 unsigned long long,
                                 struct rwlockmap*,
                                 struct picotm_error*) = {
        rwcountermap_rdlock,
        rwcountermap_wrlock
    };

    assert(self);

    lock[write](rwcountermap, reccount(nbyte), recoffset(off),
                &self->data.regular.rwlockmap, error);

    if (picotm_error_is_set(error)) {
        return;
    }
}

void
ofd_2pl_unlock_region(struct ofd* self, off_t off, size_t nbyte,
                      struct rwcountermap* rwcountermap)
{
    assert(self);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    rwcountermap_unlock(rwcountermap,
                        reccount(nbyte),
                        recoffset(off),
                        &self->data.regular.rwlockmap,
                        &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

void
ofd_dump(const struct ofd* self)
{
    fprintf(stderr, "%p: %ld\n", (const void*)self, (long)self->flags);
}
