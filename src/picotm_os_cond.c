/*
 * picotm - A system-level transaction manager
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

#include "picotm_os_cond.h"
#include <assert.h>
#include <errno.h>
#include "picotm_os_mutex.h"
#include "picotm_os_timespec.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

static void
safe_pthread_condattr_destroy(pthread_condattr_t* attr)
{
    do {
        int err = pthread_condattr_destroy(attr);
        if (err) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}

void
picotm_os_cond_init(struct picotm_os_cond* self, struct picotm_error* error)
{
    assert(self);

    pthread_condattr_t attr;

    int err = pthread_condattr_init(&attr);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

#if !defined(__MACH__)
    err = pthread_condattr_setclock(&attr, PICOTM_OS_TIMESPEC_CLOCKID);
    if (err) {
        picotm_error_set_errno(error, err);
        goto err_pthread_condattr_setclock;
    }
#endif

    err = pthread_cond_init(&self->instance, &attr);
    if (err) {
        picotm_error_set_errno(error, err);
        goto err_pthread_cond_init;
    }

    safe_pthread_condattr_destroy(&attr);

    return;

err_pthread_cond_init:
#if !defined(__MACH__)
err_pthread_condattr_setclock:
#endif
    safe_pthread_condattr_destroy(&attr);
    return;
}

void
picotm_os_cond_uninit(struct picotm_os_cond* self)
{
    assert(self);

    do {
        int err = pthread_cond_destroy(&self->instance);
        if (err) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}

void
picotm_os_cond_wait(struct picotm_os_cond* self,
                    struct picotm_os_mutex* mutex,
                    struct picotm_error* error)
{
    assert(self);
    assert(mutex);

    int err = pthread_cond_wait(&self->instance, &mutex->instance);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

bool
picotm_os_cond_wait_until(struct picotm_os_cond* self,
                          struct picotm_os_mutex* mutex,
                          const struct timespec* timeout,
                          struct picotm_error* error)
{
    assert(self);
    assert(mutex);

    int err = pthread_cond_timedwait(&self->instance, &mutex->instance,
                                     timeout);
    if (err && (err != ETIMEDOUT)) {
        picotm_error_set_errno(error, err);
        return false;
    }
    return err != ETIMEDOUT;
}

void
picotm_os_cond_wake_up(struct picotm_os_cond* self,
                       struct picotm_error* error)
{
    assert(self);

    int err = pthread_cond_signal(&self->instance);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
picotm_os_cond_wake_up_all(struct picotm_os_cond* self,
                           struct picotm_error* error)
{
    assert(self);

    int err = pthread_cond_broadcast(&self->instance);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}
