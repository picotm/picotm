/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <stddef.h>

struct event;
struct picotm_error;

struct module {
    int (*lock)(void*, struct picotm_error*);
    int (*unlock)(void*, struct picotm_error*);
    int (*validate)(void*, int, struct picotm_error*);
    int (*apply_event)(const struct event*, size_t, void*, struct picotm_error*);
    int (*undo_event)(const struct event*, size_t, void*, struct picotm_error*);
    int (*update_cc)(void*, int, struct picotm_error*);
    int (*clear_cc)(void*, int, struct picotm_error*);
    int (*finish)(void*, struct picotm_error*);
    void (*uninit)(void*);
    void *data;
};

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
            void* data);

void
module_uninit(struct module* self);

void *
module_get_data(const struct module* self);

int
module_lock(const struct module* self, struct picotm_error* error);

int
module_unlock(const struct module* self, struct picotm_error* error);

int
module_validate(const struct module* self, bool noundo,
                struct picotm_error* error);

int
module_apply_events(const struct module* self, const struct event *event,
                    size_t nevents, struct picotm_error* error);

int
module_undo_events(const struct module* self, const struct event *event,
                   size_t nevents, struct picotm_error* error);

int
module_update_cc(const struct module* self, bool noundo,
                 struct picotm_error* error);

int
module_clear_cc(const struct module* self, bool noundo,
                struct picotm_error* error);

int
module_finish(const struct module* self, struct picotm_error* error);
