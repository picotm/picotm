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
#include <picotm/picotm-lib-rwstate.h>
#include <stdlib.h>
#include <unistd.h>

static void
lock_fd(struct fd* fd)
{
    int err = pthread_mutex_lock(&fd->lock);
    if (err) {
        abort();
    }
}

static void
unlock_fd(struct fd* fd)
{
    int err = pthread_mutex_unlock(&fd->lock);
    if (err) {
        abort();
    }
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

    int err = pthread_mutex_init(&fd->lock, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    picotm_ref_init(&fd->ref, 0);

    fd->fildes = -1;
    fd->state = FD_ST_UNUSED;

    init_rwlocks(picotm_arraybeg(fd->rwlock),
                 picotm_arrayend(fd->rwlock));
}

void
fd_uninit(struct fd *fd)
{
    int err = pthread_mutex_destroy(&fd->lock);
    if (err) {
        abort();
    }

    uninit_rwlocks(picotm_arraybeg(fd->rwlock),
                   picotm_arrayend(fd->rwlock));
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

static bool
incr_ref_count(struct fd *fd, struct picotm_error* error)
{
    bool first_ref;

    switch (fd->state) {
        case FD_ST_UNUSED:
            /* fall through */
        case FD_ST_INUSE:
            first_ref = picotm_ref_up(&fd->ref);
            break;
        case FD_ST_CLOSING:
            /* fd is about to be closed; abort here */
            picotm_error_set_conflicting(error, NULL);
            return false;
        default:
            abort();
    }

    return first_ref;
}

void
fd_ref(struct fd *fd, struct picotm_error* error)
{
    incr_ref_count(fd, error);
}

void
fd_ref_or_set_up(struct fd *fd, int fildes, struct picotm_error* error)
{
    lock_fd(fd);

    bool first_ref = incr_ref_count(fd, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    if (!first_ref) {
        /* we got a set-up instance; signal success */
        goto unlock;
    }

    /* set up instance */
    fd->fildes = fildes;
    fd->state = FD_ST_INUSE;

unlock:
    unlock_fd(fd);
}

static void
fd_cleanup(struct fd *fd)
{
    assert(fd);

    fd->fildes = -1;
    fd->state = FD_ST_UNUSED;
}

void
fd_unref(struct fd *fd)
{
    assert(fd);

    lock_fd(fd);

    bool final_ref = picotm_ref_down(&fd->ref);
    if (!final_ref) {
        goto unlock;
    }

    switch (fd->state) {
        case FD_ST_UNUSED:
        case FD_ST_INUSE: {
            fd_cleanup(fd);
            break;
        }
        case FD_ST_CLOSING: {
            /* close fildes if we released the final reference */
            int res = TEMP_FAILURE_RETRY(close(fd->fildes));
            if (res < 0) {
                abort(); /* FIXME: Raise error flag */
            }

            fd_cleanup(fd);
            break;
        }
        default:
            abort(); /* should never happen */
    }

unlock:
    unlock_fd(fd);
}

void
fd_close(struct fd *fd)
{
    assert(fd);

    fd->state = FD_ST_CLOSING;
}

void
fd_dump(const struct fd *fd)
{
    return;
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
