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

#include "fd.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-ptr.h>
#include <picotm/picotm-lib-rwstate.h>
#include <stdlib.h>
#include <unistd.h>

static struct fd*
fd_of_picotm_shared_ref16_obj(struct picotm_shared_ref16_obj* ref_obj)
{
    return picotm_containerof(ref_obj, struct fd, ref_obj);
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
fd_init(struct fd *fd, struct picotm_error* error)
{
    assert(fd);

    picotm_shared_ref16_obj_init(&fd->ref_obj, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    fd->fildes = -1;
    fd->state = FD_ST_UNUSED;

    init_rwlocks(picotm_arraybeg(fd->rwlock),
                 picotm_arrayend(fd->rwlock));
}

void
fd_uninit(struct fd *fd)
{
    assert(fd);

    uninit_rwlocks(picotm_arraybeg(fd->rwlock),
                   picotm_arrayend(fd->rwlock));

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_shared_ref16_obj_uninit(&fd->ref_obj, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

void
fd_try_rdlock_field(struct fd *fd, enum fd_field field,
                    struct picotm_rwstate* rwstate,
                    struct picotm_error* error)
{
    assert(fd);

    picotm_rwstate_try_rdlock(rwstate, fd->rwlock + field, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_try_wrlock_field(struct fd *fd, enum fd_field field,
                    struct picotm_rwstate* rwstate,
                    struct picotm_error* error)
{
    assert(fd);

    picotm_rwstate_try_wrlock(rwstate, fd->rwlock + field, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_unlock_field(struct fd *fd, enum fd_field field,
                struct picotm_rwstate* rwstate)
{
    assert(fd);

    picotm_rwstate_unlock(rwstate, fd->rwlock + field);
}

int
fd_is_open_nl(const struct fd *fd)
{
	assert(fd);

	return fd->state == FD_ST_INUSE;
}

void
fd_validate(struct fd* fd, struct picotm_error* error)
{
    assert(fd);
}

/*
 * Referencing
 */

struct ref_obj_data {
    int fildes;
};

static bool
cond_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
         struct picotm_error* error)
{
    struct fd* self = fd_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    if (self->state == FD_ST_CLOSING) {
        /* fd is about to be closed; don't allow new references */
        picotm_error_set_conflicting(error, NULL);
        return false;
    }

    return true;
}

static void
first_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct fd* self = fd_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    const struct ref_obj_data* ref_obj_data = data;
    assert(ref_obj_data);

    /* set up instance */
    self->fildes = ref_obj_data->fildes;
    self->state = FD_ST_INUSE;
}

void
fd_ref(struct fd *fd, struct picotm_error* error)
{
    assert(fd);

    struct ref_obj_data data = {
        -1
    };

    picotm_shared_ref16_obj_up(&fd->ref_obj, &data, cond_ref, NULL, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_ref_or_set_up(struct fd *fd, int fildes, struct picotm_error* error)
{
    assert(fd);

    struct ref_obj_data data = {
        fildes
    };

    picotm_shared_ref16_obj_up(&fd->ref_obj, &data, cond_ref, first_ref, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
final_ref(struct picotm_shared_ref16_obj* ref_obj, void* data,
          struct picotm_error* error)
{
    struct fd* self = fd_of_picotm_shared_ref16_obj(ref_obj);
    assert(self);

    if (self->state == FD_ST_CLOSING) {
        /* Close fildes if we released the final reference of
         * a file descriptor that has been closed from within
         * a transaction. */
        int res = TEMP_FAILURE_RETRY(close(self->fildes));
        if (res < 0) {
            picotm_error_set_errno(error, errno);
            picotm_error_mark_as_non_recoverable(error);
            return;
        }
    }

    self->fildes = -1;
    self->state = FD_ST_UNUSED;
}

void
fd_unref(struct fd *fd)
{
    assert(fd);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_shared_ref16_obj_down(&fd->ref_obj, NULL, NULL, final_ref, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

void
fd_close(struct fd *fd)
{
    assert(fd);

    fd->state = FD_ST_CLOSING;
}

int
fd_setfd(struct fd* fd, int arg, struct picotm_error* error)
{
    assert(fd);

    int res = TEMP_FAILURE_RETRY(fcntl(fd->fildes, F_SETFD, arg));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
fd_getfd(struct fd* fd, struct picotm_error* error)
{
    assert(fd);

    int res = TEMP_FAILURE_RETRY(fcntl(fd->fildes, F_GETFD));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}
