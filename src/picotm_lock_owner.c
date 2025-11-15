/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
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

#include "picotm_lock_owner.h"
#include <assert.h>
#include "picotm_os_timespec.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

void
picotm_lock_owner_init(struct picotm_lock_owner* self,
                       struct picotm_error* error)
{
    assert(self);

    picotm_os_mutex_init(&self->mutex, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    picotm_os_cond_init(&self->wait_cond, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_os_cond_init;
    }

    self->flags = 0;
    self->next = NULL;

    return;

err_picotm_os_cond_init:
    picotm_os_mutex_uninit(&self->mutex);
}

void
picotm_lock_owner_uninit(struct picotm_lock_owner* self)
{
    assert(self);

    picotm_os_mutex_uninit(&self->mutex);
    picotm_os_cond_uninit(&self->wait_cond);
}

void
picotm_lock_owner_set_index(struct picotm_lock_owner* self,
                            unsigned long index)
{
    assert(self);
    assert(index < (1ul << 10));

    self->flags = (self->flags & ~0x3ff) | (index & 0x3ff);
}

unsigned long
picotm_lock_owner_get_index(const struct picotm_lock_owner* self)
{
    assert(self);

    return self->flags & 0x3ff;
}

void
picotm_lock_owner_set_next(struct picotm_lock_owner* self,
                           unsigned long next)
{
    assert(self);
    assert(next < (1ul << 10));

    self->flags = (self->flags & ~0xffc00) | ((next & 0x3ff) << 10);
}

unsigned long
picotm_lock_owner_get_next(const struct picotm_lock_owner* self)
{
    assert(self);

    return (self->flags >> 10) & 0x3ff;
}

const struct timespec*
picotm_lock_owner_get_timestamp(const struct picotm_lock_owner* self)
{
    assert(self);

    return &self->timestamp;
}

void
picotm_lock_owner_reset_timestamp(struct picotm_lock_owner* self,
                                  struct picotm_error* error)
{
    assert(self);

    picotm_os_get_timespec(&self->timestamp, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
picotm_lock_owner_lock(struct picotm_lock_owner* self,
                       struct picotm_error* error)
{
    assert(self);

    picotm_os_mutex_lock(&self->mutex, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
picotm_lock_owner_unlock(struct picotm_lock_owner* self)
{
    assert(self);

    picotm_os_mutex_unlock(&self->mutex);
}

bool
picotm_lock_owner_wait_until(struct picotm_lock_owner* self,
                             const struct timespec* timeout,
                             struct picotm_error* error)
{
    assert(self);
    assert(!(self->flags & LOCK_OWNER_WT));

    /* set waiting flag */
    self->flags |= LOCK_OWNER_WT;

    bool woken_up;

    do {
        woken_up = picotm_os_cond_wait_until(&self->wait_cond,
                                             &self->mutex,
                                             timeout, error);
        if (picotm_error_is_set(error)) {
            goto err_picotm_os_cond_wait_until;
        }

        /* Platforms can have spurious wake-ups from condition
         * variables. The presence of the waiting flag signals
         * a spurious wakeup. */
    } while (woken_up && (self->flags & LOCK_OWNER_WT));

    if (!woken_up) {
        /* clear waiting flag */
        self->flags &= ~LOCK_OWNER_WT;
    }

    return woken_up;

err_picotm_os_cond_wait_until:
    /* clear waiting flag */
    self->flags &= ~LOCK_OWNER_WT;
    return false;
}

void
picotm_lock_owner_wake_up(struct picotm_lock_owner* self,
                          struct picotm_error* error)
{
    assert(self);
    assert(self->flags & LOCK_OWNER_WT);

    /* clear waiting flag */
    self->flags &= ~LOCK_OWNER_WT;

    picotm_os_cond_wake_up(&self->wait_cond, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}
