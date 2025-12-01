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

#include "picotm_os_mutex.h"
#include <assert.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

void
picotm_os_mutex_init(struct picotm_os_mutex* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_mutex_init(&self->instance, nullptr);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
picotm_os_mutex_uninit(struct picotm_os_mutex* self)
{
    assert(self);

    do {
        int err = pthread_mutex_destroy(&self->instance);
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
picotm_os_mutex_lock(struct picotm_os_mutex* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_mutex_lock(&self->instance);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
picotm_os_mutex_unlock(struct picotm_os_mutex* self)
{
    assert(self);

    do {
        int err = pthread_mutex_unlock(&self->instance);
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
