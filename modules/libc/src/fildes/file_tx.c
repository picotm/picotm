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
#include <assert.h>
#include "file_tx_ops.h"

void
file_tx_init(struct file_tx* self,  const struct file_tx_ops* ops)
{
    assert(self);
    assert(ops);

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

void
file_tx_ref(struct file_tx* self, struct picotm_error* error)
{
    assert(self);

    self->ops->ref(self, error);
}

void
file_tx_unref(struct file_tx* self)
{
    assert(self);

    self->ops->unref(self);
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
