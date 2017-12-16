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

#include "picotm/picotm-lib-rwstate.h"
#include <assert.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-rwlock.h"

PICOTM_EXPORT
void
picotm_rwstate_init(struct picotm_rwstate* self)
{
    assert(self);

    self->status = PICOTM_RWSTATE_UNLOCKED;
}

PICOTM_EXPORT
void
picotm_rwstate_uninit(struct picotm_rwstate* self)
{
    assert(self);
}

PICOTM_EXPORT
void
picotm_rwstate_set_status(struct picotm_rwstate* self,
                          enum picotm_rwstate_status status)
{
    assert(self);

    self->status = status;
}

PICOTM_EXPORT
enum picotm_rwstate_status
picotm_rwstate_get_status(const struct picotm_rwstate* self)
{
    assert(self);

    return self->status;
}

PICOTM_EXPORT
void
picotm_rwstate_try_rdlock(struct picotm_rwstate* self,
                          struct picotm_rwlock* rwlock,
                          struct picotm_error* error)
{
    assert(self);

    if (self->status != PICOTM_RWSTATE_UNLOCKED) {
        return; /* already locked; nothing to do */
    }

    picotm_rwlock_try_rdlock(rwlock, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->status = PICOTM_RWSTATE_RDLOCKED;
}

PICOTM_EXPORT
void
picotm_rwstate_try_wrlock(struct picotm_rwstate* self,
                          struct picotm_rwlock* rwlock,
                          struct picotm_error* error)
{
    assert(self);

    if (self->status == PICOTM_RWSTATE_WRLOCKED) {
        return; /* already write locked; nothing to do */
    }

    picotm_rwlock_try_wrlock(rwlock, self->status == PICOTM_RWSTATE_RDLOCKED,
                             error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->status = PICOTM_RWSTATE_WRLOCKED;
}

PICOTM_EXPORT
void
picotm_rwstate_unlock(struct picotm_rwstate* self,
                      struct picotm_rwlock* rwlock)
{
    assert(self);

    if (self->status == PICOTM_RWSTATE_UNLOCKED) {
        return;
    }

    picotm_rwlock_unlock(rwlock);
    self->status = PICOTM_RWSTATE_UNLOCKED;
}
