/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
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

#include "fd.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-rwstate.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "compat/temp_failure_retry.h"

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
fd_init(struct fd* self, struct picotm_error* error)
{
    assert(self);

    picotm_shared_ref16_obj_init(&self->ref_obj, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->fildes = -1;
    self->state = FD_STATE_UNUSED;

    init_rwlocks(picotm_arraybeg(self->rwlock),
                 picotm_arrayend(self->rwlock));
}

void
fd_uninit(struct fd* self)
{
    assert(self);

    uninit_rwlocks(picotm_arraybeg(self->rwlock),
                   picotm_arrayend(self->rwlock));

    picotm_shared_ref16_obj_uninit(&self->ref_obj);
}

void
fd_try_rdlock_field(struct fd* self, enum fd_field field,
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
fd_try_wrlock_field(struct fd* self, enum fd_field field,
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
fd_unlock_field(struct fd* self, enum fd_field field,
                struct picotm_rwstate* rwstate)
{
    assert(self);

    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}

int
fd_is_open_nl(const struct fd* self)
{
	assert(self);

	return self->state == FD_STATE_INUSE;
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

    if (self->state == FD_STATE_CLOSING) {
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
    self->state = FD_STATE_INUSE;
}

void
fd_ref(struct fd* self, struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = {
        -1
    };

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, NULL, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fd_ref_or_set_up(struct fd* self, int fildes, struct picotm_error* error)
{
    assert(self);

    struct ref_obj_data data = {
        fildes
    };

    picotm_shared_ref16_obj_up(&self->ref_obj, &data, cond_ref, first_ref, error);
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

    if (self->state == FD_STATE_CLOSING) {
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
    self->state = FD_STATE_UNUSED;
}

void
fd_unref(struct fd* self)
{
    assert(self);

    picotm_shared_ref16_obj_down(&self->ref_obj, NULL, NULL, final_ref);
}

void
fd_close(struct fd* self)
{
    assert(self);

    self->state = FD_STATE_CLOSING;
}

int
fd_setfd(struct fd* self, int arg, struct picotm_error* error)
{
    assert(self);

    int res = TEMP_FAILURE_RETRY(fcntl(self->fildes, F_SETFD, arg));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}

int
fd_getfd(struct fd* self, struct picotm_error* error)
{
    assert(self);

    int res = TEMP_FAILURE_RETRY(fcntl(self->fildes, F_GETFD));
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        return res;
    }
    return res;
}
