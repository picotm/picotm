/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
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
