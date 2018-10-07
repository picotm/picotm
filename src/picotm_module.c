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

#include "picotm_module.h"
#include <assert.h>
#include "picotm/picotm-module.h"

void
picotm_module_init(struct picotm_module* self,
                   const struct picotm_module_ops* ops,
                   void *data)
{
    assert(self);

    self->ops = ops;
    self->data = data;
}

void
picotm_module_release(struct picotm_module* self)
{
    assert(self);
    assert(self->ops);

    if (self->ops->release) {
        self->ops->release(self->data);
    }
}

void*
picotm_module_get_data(const struct picotm_module* self)
{
    assert(self);

    return self->data;
}

void
picotm_module_begin(const struct picotm_module* self,
                    struct picotm_error* error)
{
    assert(self);
    assert(self->ops);

    if (!self->ops->begin) {
        return;
    }
    self->ops->begin(self->data, error);
}

void
picotm_module_prepare_commit(const struct picotm_module* self, bool noundo,
                             struct picotm_error* error)
{
    assert(self);
    assert(self->ops);

    if (!self->ops->prepare_commit) {
        return;
    }
    self->ops->prepare_commit(self->data, noundo, error);
}

void
picotm_module_apply(const struct picotm_module* self,
                    struct picotm_error* error)
{
    assert(self);
    assert(self->ops);

    if (!self->ops->apply) {
        return;
    }
    self->ops->apply(self->data, error);
}

void
picotm_module_undo(const struct picotm_module* self,
                   struct picotm_error* error)
{
    assert(self);
    assert(self->ops);

    if (!self->ops->undo) {
        return;
    }
    self->ops->undo(self->data, error);
}

void
picotm_module_apply_event(const struct picotm_module* self,
                          uint16_t head, uintptr_t tail,
                          struct picotm_error* error)
{
    assert(self);
    assert(self->ops);

    if (!self->ops->apply_event) {
        return;
    }
    self->ops->apply_event(head, tail, self->data, error);
}

void
picotm_module_undo_event(const struct picotm_module* self,
                         uint16_t head, uintptr_t tail,
                         struct picotm_error* error)
{
    assert(self);
    assert(self->ops);

    if (!self->ops->undo_event) {
        return;
    }
    self->ops->undo_event(head, tail, self->data, error);
}

void
picotm_module_finish(const struct picotm_module* self,
                     struct picotm_error* error)
{
    assert(self);
    assert(self->ops);

    if (!self->ops->finish) {
        return;
    }
    self->ops->finish(self->data, error);
}
