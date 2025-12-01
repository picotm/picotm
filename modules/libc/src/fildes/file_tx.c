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

#include "file_tx.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include "file.h"
#include "file_tx_ops.h"

void
file_tx_init(struct file_tx* self,  const struct file_tx_ops* ops)
{
    assert(self);
    assert(ops);

    picotm_ref_init(&self->ref, 0);
    picotm_slist_init_item(&self->list_entry);
    self->file = nullptr;
    self->ops = ops;
}

void
file_tx_uninit(struct file_tx* self)
{
    assert(self);
    assert(!self->file);

    picotm_slist_uninit_item(&self->list_entry);
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
file_tx_ref_or_set_up(struct file_tx* self, struct file* file,
                      void* data, struct picotm_error* error)
{
    assert(self);
    assert(!self->file);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on shared file state */
    file_ref(file, error);
    if (picotm_error_is_set(error)) {
        goto err_file_ref;
    }
    self->file = file;

    /* prepare file-type sub class */
    self->ops->prepare(self, file, data, error);
    if (picotm_error_is_set(error)) {
        goto err_self_ops_acquire_file;
    }

    return;

err_self_ops_acquire_file:
    self->file = nullptr;
    file_unref(file);
err_file_ref:
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
    assert(self->file);

    bool final_unref = picotm_ref_down(&self->ref);
    if (!final_unref) {
        return;
    }

    self->ops->release(self);

    file_unref(self->file);
    self->file = nullptr;
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
