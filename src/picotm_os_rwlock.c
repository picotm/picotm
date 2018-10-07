/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm_os_rwlock.h"
#include <assert.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

void
picotm_os_rwlock_init(struct picotm_os_rwlock* self,
                      struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_init(&self->instance, NULL);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
picotm_os_rwlock_uninit(struct picotm_os_rwlock* self)
{
    assert(self);

    do {
        int err = pthread_rwlock_destroy(&self->instance);
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
picotm_os_rwlock_rdlock(struct picotm_os_rwlock* self,
                        struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_rdlock(&self->instance);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
picotm_os_rwlock_wrlock(struct picotm_os_rwlock* self,
                        struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_wrlock(&self->instance);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
picotm_os_rwlock_unlock(struct picotm_os_rwlock* self)
{
    assert(self);

    do {
        int err = pthread_rwlock_unlock(&self->instance);
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
