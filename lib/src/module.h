/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <stddef.h>

struct event;

struct module {
    int (*lock)(void*);
    int (*unlock)(void*);
    int (*validate)(void*, int);
    int (*apply_event)(const struct event*, size_t, void*);
    int (*undo_event)(const struct event*, size_t, void*);
    int (*update_cc)(void*, int);
    int (*clear_cc)(void*, int);
    int (*finish)(void*);
    int (*uninit)(void*);
    void *data;
};

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
            void* data);

void
module_uninit(struct module* self);

void *
module_get_data(const struct module* self);

int
module_lock(const struct module* self);

int
module_unlock(const struct module* self);

int
module_validate(const struct module* self, bool noundo);

int
module_apply_events(const struct module* self, const struct event *event,
                    size_t nevents);

int
module_undo_events(const struct module* self, const struct event *event,
                   size_t nevents);

int
module_update_cc(const struct module* self, bool noundo);

int
module_clear_cc(const struct module* self, bool noundo);

int
module_finish(const struct module* self);
