/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"

int
module_init(struct module* self,
            int (*lock)(void*),
            int (*unlock)(void*),
            int (*validate)(void*, int),
            int (*apply_event)(const struct event*, size_t, void*),
            int (*undo_event)(const struct event*, size_t, void*),
            int (*update_cc)(void*, int),
            int (*clear_cc)(void*, int),
            int (*finish)(void*),
            int (*uninit)(void*),
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
module_lock(const struct module* self)
{
    return self->lock ? self->lock(self->data) : 0;
}

int
module_unlock(const struct module* self)
{
    return self->unlock ? self->unlock(self->data) : 0;
}

int
module_validate(const struct module* self, bool noundo)
{
    return self->validate ? self->validate(self->data, noundo) : 0;
}

int
module_apply_events(const struct module* self, const struct event* event,
                    size_t nevents)
{
    return self->apply_event ?
        self->apply_event(event, nevents, self->data) : 0;
}

int
module_undo_events(const struct module* self, const struct event* event,
                   size_t nevents)
{
    return self->undo_event ?
        self->undo_event(event, nevents, self->data) : 0;
}

int
module_update_cc(const struct module* self, bool noundo)
{
    return self->update_cc ? self->update_cc(self->data, noundo) : 0;
}

int
module_clear_cc(const struct module* self, bool noundo)
{
    return self->clear_cc ? self->clear_cc(self->data, noundo) : 0;
}

int
module_finish(const struct module* self)
{
    return self->finish ? self->finish(self->data) : 0;
}
