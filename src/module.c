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

#include "module.h"
#include <assert.h>

void
module_init(struct module* self,
            void (*lock)(void*, struct picotm_error*),
            void (*unlock)(void*, struct picotm_error*),
            void (*validate)(void*, int, struct picotm_error*),
            void (*apply)(void*, struct picotm_error*),
            void (*undo)(void*, struct picotm_error*),
            void (*apply_event)(uint16_t, uintptr_t, void*,
                                struct picotm_error*),
            void (*undo_event)(uint16_t, uintptr_t, void*,
                               struct picotm_error*),
            void (*update_cc)(void*, int, struct picotm_error*),
            void (*clear_cc)(void*, int, struct picotm_error*),
            void (*finish)(void*, struct picotm_error*),
            void (*uninit)(void*),
            void *data)
{
    assert(self);

    self->lock = lock;
    self->unlock = unlock;
    self->validate = validate;
    self->apply = apply;
    self->undo = undo;
    self->apply_event = apply_event;
    self->undo_event = undo_event;
    self->update_cc = update_cc;
    self->clear_cc = clear_cc;
    self->finish = finish;
    self->uninit = uninit;
    self->data = data;
}

void
module_uninit(struct module* self)
{
    assert(self);

    if (self->uninit) {
        self->uninit(self->data);
    }
}

void*
module_get_data(const struct module* self)
{
    assert(self);

    return self->data;
}

void
module_lock(const struct module* self, struct picotm_error* error)
{
    assert(self);

    if (!self->lock) {
        return;
    }
    self->lock(self->data, error);
}

void
module_unlock(const struct module* self, struct picotm_error* error)
{
    assert(self);

    if (!self->unlock) {
        return;
    }
    self->unlock(self->data, error);
}

void
module_validate(const struct module* self, bool noundo,
                struct picotm_error* error)
{
    assert(self);

    if (!self->validate) {
        return;
    }
    self->validate(self->data, noundo, error);
}

void
module_apply(const struct module* self, struct picotm_error* error)
{
    assert(self);

    if (!self->apply) {
        return;
    }
    self->apply(self->data, error);
}

void
module_undo(const struct module* self, struct picotm_error* error)
{
    assert(self);

    if (!self->undo) {
        return;
    }
    self->undo(self->data, error);
}

void
module_apply_event(const struct module* self, uint16_t head, uintptr_t tail,
                   struct picotm_error* error)
{
    assert(self);

    if (!self->apply_event) {
        return;
    }
    self->apply_event(head, tail, self->data, error);
}

void
module_undo_event(const struct module* self, uint16_t head, uintptr_t tail,
                  struct picotm_error* error)
{
    assert(self);

    if (!self->undo_event) {
        return;
    }
    self->undo_event(head, tail, self->data, error);
}

void
module_update_cc(const struct module* self, bool noundo,
                 struct picotm_error* error)
{
    assert(self);

    if (!self->update_cc) {
        return;
    }
    self->update_cc(self->data, noundo, error);
}

void
module_clear_cc(const struct module* self, bool noundo,
                struct picotm_error* error)
{
    assert(self);

    if (!self->clear_cc) {
        return;
    }
    self->clear_cc(self->data, noundo, error);
}

void
module_finish(const struct module* self, struct picotm_error* error)
{
    assert(self);

    if (!self->finish) {
        return;
    }
    self->finish(self->data, error);
}
