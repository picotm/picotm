/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "file_tx.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include "file_tx_ops.h"

void
file_tx_init(struct file_tx* self,  const struct file_tx_ops* ops)
{
    assert(self);
    assert(ops);

    picotm_ref_init(&self->ref, 0);
    picotm_slist_init_item(&self->active_list);
    self->ops = ops;
}

void
file_tx_uninit(struct file_tx* self)
{
    picotm_slist_uninit_item(&self->active_list);
}

enum picotm_libc_file_type
file_tx_file_type(const struct file_tx* self)
{
    assert(self);

    return self->ops->type;
}

/*
 * Referencing
 */

void
file_tx_ref_or_set_up(struct file_tx* self, void* file,
                      struct picotm_error* error)
{
    assert(self);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    self->ops->acquire_file(self, file, error);
    if (picotm_error_is_set(error)) {
        goto err_self_ops_acquire_file;
    }

    return;

err_self_ops_acquire_file:
    picotm_ref_down(&self->ref);
}

void
file_tx_ref(struct file_tx* self, struct picotm_error* error)
{
    picotm_ref_up(&self->ref);
}

void
file_tx_unref(struct file_tx* self)
{
    assert(self);

    bool final_unref = picotm_ref_down(&self->ref);
    if (!final_unref) {
        return;
    }

    self->ops->release_file(self);
}

bool
file_tx_holds_ref(struct file_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

/*
 * Module interfaces
 */

void
file_tx_finish(struct file_tx* self)
{
    assert(self);
    assert(self->ops);
    assert(self->ops->finish);

    self->ops->finish(self);
}
