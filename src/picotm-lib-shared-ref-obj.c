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
