/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
