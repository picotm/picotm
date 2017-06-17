/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"

void
module_init(struct module* self,
            void (*lock)(void*, struct picotm_error*),
            void (*unlock)(void*, struct picotm_error*),
            bool (*is_valid)(void*, int, struct picotm_error*),
            void (*apply)(void*, struct picotm_error*),
            void (*undo)(void*, struct picotm_error*),
            void (*apply_event)(const struct picotm_event*,
                                void*, struct picotm_error*),
            void (*undo_event)(const struct picotm_event*,
                               void*, struct picotm_error*),
            void (*update_cc)(void*, int, struct picotm_error*),
            void (*clear_cc)(void*, int, struct picotm_error*),
            void (*finish)(void*, struct picotm_error*),
            void (*uninit)(void*),
            void *data)
{
    self->lock = lock;
    self->unlock = unlock;
    self->is_valid = is_valid;
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
    if (self->uninit) {
        self->uninit(self->data);
    }
}

void*
module_get_data(const struct module* self)
{
    return self->data;
}

void
module_lock(const struct module* self, struct picotm_error* error)
{
    if (!self->lock) {
        return;
    }
    self->lock(self->data, error);
}

void
module_unlock(const struct module* self, struct picotm_error* error)
{
    if (!self->unlock) {
        return;
    }
    self->unlock(self->data, error);
}

bool
module_is_valid(const struct module* self, bool noundo,
                struct picotm_error* error)
{
    if (!self->is_valid) {
        return true;
    }
    return self->is_valid(self->data, noundo, error);
}

void
module_apply(const struct module* self, struct picotm_error* error)
{
    if (!self->apply) {
        return;
    }
    self->apply(self->data, error);
}

void
module_undo(const struct module* self, struct picotm_error* error)
{
    if (!self->undo) {
        return;
    }
    self->undo(self->data, error);
}

void
module_apply_event(const struct module* self, const struct picotm_event* event,
                   struct picotm_error* error)
{
    if (!self->apply_event) {
        return;
    }
    self->apply_event(event, self->data, error);
}

void
module_undo_event(const struct module* self, const struct picotm_event* event,
                  struct picotm_error* error)
{
    if (!self->undo_event) {
        return;
    }
    self->undo_event(event, self->data, error);
}

void
module_update_cc(const struct module* self, bool noundo,
                 struct picotm_error* error)
{
    if (!self->update_cc) {
        return;
    }
    self->update_cc(self->data, noundo, error);
}

void
module_clear_cc(const struct module* self, bool noundo,
                struct picotm_error* error)
{
    if (!self->clear_cc) {
        return;
    }
    self->clear_cc(self->data, noundo, error);
}

void
module_finish(const struct module* self, struct picotm_error* error)
{
    if (!self->finish) {
        return;
    }
    self->finish(self->data, error);
}
