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

#include "picotm_os_rwlock.h"
#include <assert.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

void
picotm_os_rwlock_init(struct picotm_os_rwlock* self,
                      struct picotm_error* error)
{
    assert(self);

    int err = pthread_rwlock_init(&self->instance, nullptr);
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
