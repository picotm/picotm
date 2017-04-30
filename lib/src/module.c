/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"

int
module_init(struct module* self,
            int (*lock)(void*, struct picotm_error*),
            int (*unlock)(void*, struct picotm_error*),
            int (*validate)(void*, int, struct picotm_error*),
            int (*apply_event)(const struct event*, size_t, void*, struct picotm_error*),
            int (*undo_event)(const struct event*, size_t, void*, struct picotm_error*),
            int (*update_cc)(void*, int, struct picotm_error*),
            int (*clear_cc)(void*, int, struct picotm_error*),
            int (*finish)(void*, struct picotm_error*),
            void (*uninit)(void*),
            void *data)
{
    self->lock = lock;
    self->unlock = unlock;
    self->validate = validate;
    self->apply_event = apply_event;
    self->undo_event = undo_event;
    self->update_cc = update_cc;
    self->clear_cc = clear_cc;
    self->finish = finish;
    self->uninit = uninit;
    self->data = data;

    return 0;
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

int
module_lock(const struct module* self, struct picotm_error* error)
{
    if (!self->lock) {
        return 0;
    }
    return self->lock(self->data, error);
}

int
module_unlock(const struct module* self, struct picotm_error* error)
{
    if (!self->unlock) {
        return 0;
    }
    return self->unlock(self->data, error);
}

int
module_validate(const struct module* self, bool noundo,
                struct picotm_error* error)
{
    if (!self->validate) {
        return 0;
    }
    return self->validate(self->data, noundo, error);
}

int
module_apply_events(const struct module* self, const struct event* event,
                    size_t nevents, struct picotm_error* error)
{
    if (!self->apply_event) {
        return 0;
    }
    return self->apply_event(event, nevents, self->data, error);
}

int
module_undo_events(const struct module* self, const struct event* event,
                   size_t nevents, struct picotm_error* error)
{
    if (!self->undo_event) {
        return 0;
    }
    return self->undo_event(event, nevents, self->data, error);
}

int
module_update_cc(const struct module* self, bool noundo,
                 struct picotm_error* error)
{
    if (!self->update_cc) {
        return 0;
    }
    return self->update_cc(self->data, noundo, error);
}

int
module_clear_cc(const struct module* self, bool noundo,
                struct picotm_error* error)
{
    if (!self->clear_cc) {
        return 0;
    }
    return self->clear_cc(self->data, noundo, error);
}

int
module_finish(const struct module* self, struct picotm_error* error)
{
    if (!self->finish) {
        return 0;
    }
    return self->finish(self->data, error);
}
