/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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
 *
 * SPDX-License-Identifier: MIT
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
