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

#include "picotm/picotm-lib-shared-ref-obj.h"
#include <stdlib.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

PICOTM_EXPORT
void
picotm_shared_ref16_obj_init(struct picotm_shared_ref16_obj* self,
                             struct picotm_error* error)
{
    assert(self);

    picotm_spinlock_init(&self->lock);
    picotm_ref_init(&self->ref, 0);
}

PICOTM_EXPORT
void
picotm_shared_ref16_obj_uninit(struct picotm_shared_ref16_obj* self)
{
    assert(self);

    picotm_spinlock_uninit(&self->lock);
}

PICOTM_EXPORT
void
picotm_shared_ref16_obj_up(struct picotm_shared_ref16_obj* self, void* data,
                           picotm_shared_ref16_obj_condition_function cond,
                           picotm_shared_ref16_obj_first_ref_function first_ref,
                           struct picotm_error* error)
{
    assert(self);

    if (!cond && !first_ref) {
        /* fast path: no condition or init; only increment counter */
        picotm_ref_up(&self->ref);
        return;
    }

    picotm_spinlock_lock(&self->lock);

    if (cond) {
        bool succ = cond(self, data, error);
        if (picotm_error_is_set(error)) {
            goto err_cond;
        } else if (!succ) {
            goto unlock;
        }
    }

    bool is_first_ref = picotm_ref_up(&self->ref);
    if (!is_first_ref) {
        /* we got a set-up instance; signal success */
        goto unlock;
    }

    if (first_ref) {
        first_ref(self, data, error);
        if (picotm_error_is_set(error)) {
            goto err_first_ref;
        }
    }

unlock:
    picotm_spinlock_unlock(&self->lock);

    return;

err_first_ref:
    picotm_ref_down(&self->ref);
err_cond:
    picotm_spinlock_unlock(&self->lock);
}

PICOTM_EXPORT
void
picotm_shared_ref16_obj_down(struct picotm_shared_ref16_obj* self, void* data,
                             picotm_shared_ref16_obj_condition_function cond,
                             picotm_shared_ref16_obj_final_ref_function final_ref)
{
    assert(self);

    picotm_spinlock_lock(&self->lock);

    if (cond) {
        do {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            bool succ = cond(self, data, &error);
            if (picotm_error_is_set(&error)) {
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
                continue;
            } else if (!succ) {
                goto unlock;
            }
            break;
        } while (true);
    }

    bool is_final_ref = picotm_ref_down(&self->ref);
    if (!is_final_ref) {
        goto unlock;
    }

    if (final_ref) {
        do {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            final_ref(self, data, &error);
            if (picotm_error_is_set(&error)) {
                picotm_error_mark_as_non_recoverable(&error);
                picotm_recover_from_error(&error);
                continue;
            }
            break;
        } while (true);
    }

unlock:
    picotm_spinlock_unlock(&self->lock);
}

PICOTM_EXPORT
uint16_t
picotm_shared_ref16_obj_count(struct picotm_shared_ref16_obj* self)
{
    assert(self);

    return picotm_ref_count(&self->ref);
}
