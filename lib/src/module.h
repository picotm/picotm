/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include <stddef.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct event;
struct picotm_error;

struct module {
    void (*lock)(void*, struct picotm_error*);
    void (*unlock)(void*, struct picotm_error*);
    bool (*is_valid)(void*, int, struct picotm_error*);
    void (*apply)(void*, struct picotm_error*);
    void (*undo)(void*, struct picotm_error*);
    void (*apply_events)(const struct event*, size_t, void*, struct picotm_error*);
    void (*undo_events)(const struct event*, size_t, void*, struct picotm_error*);
    void (*update_cc)(void*, int, struct picotm_error*);
    void (*clear_cc)(void*, int, struct picotm_error*);
    void (*finish)(void*, struct picotm_error*);
    void (*uninit)(void*);
    void *data;
};

void
module_init(struct module* self,
            void (*lock)(void*, struct picotm_error*),
            void (*unlock)(void*, struct picotm_error*),
            bool (*is_valid)(void*, int, struct picotm_error*),
            void (*apply)(void*, struct picotm_error*),
            void (*undo)(void*, struct picotm_error*),
            void (*apply_events)(const struct event*, size_t, void*, struct picotm_error*),
            void (*undo_events)(const struct event*, size_t, void*, struct picotm_error*),
            void (*update_cc)(void*, int, struct picotm_error*),
            void (*clear_cc)(void*, int, struct picotm_error*),
            void (*finish)(void*, struct picotm_error*),
            void (*uninit)(void*),
            void* data);

void
module_uninit(struct module* self);

void *
module_get_data(const struct module* self);

void
module_lock(const struct module* self, struct picotm_error* error);

void
module_unlock(const struct module* self, struct picotm_error* error);

bool
module_is_valid(const struct module* self, bool noundo,
                struct picotm_error* error);

void
module_apply(const struct module* self, struct picotm_error* error);

void
module_undo(const struct module* self, struct picotm_error* error);

void
module_apply_events(const struct module* self, const struct event *event,
                    size_t nevents, struct picotm_error* error);

void
module_undo_events(const struct module* self, const struct event *event,
                   size_t nevents, struct picotm_error* error);

void
module_update_cc(const struct module* self, bool noundo,
                 struct picotm_error* error);

void
module_clear_cc(const struct module* self, bool noundo,
                struct picotm_error* error);

void
module_finish(const struct module* self, struct picotm_error* error);
