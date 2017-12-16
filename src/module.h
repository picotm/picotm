/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_event;

struct module {
    void (*lock)(void*, struct picotm_error*);
    void (*unlock)(void*, struct picotm_error*);
    void (*validate)(void*, int, struct picotm_error*);
    void (*apply)(void*, struct picotm_error*);
    void (*undo)(void*, struct picotm_error*);
    void (*apply_event)(const struct picotm_event*,
                        void*, struct picotm_error*);
    void (*undo_event)(const struct picotm_event*,
                       void*, struct picotm_error*);
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
            void (*validate)(void*, int, struct picotm_error*),
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
            void* data);

void
module_uninit(struct module* self);

void *
module_get_data(const struct module* self);

void
module_lock(const struct module* self, struct picotm_error* error);

void
module_unlock(const struct module* self, struct picotm_error* error);

void
module_validate(const struct module* self, bool noundo,
                struct picotm_error* error);

void
module_apply(const struct module* self, struct picotm_error* error);

void
module_undo(const struct module* self, struct picotm_error* error);

void
module_apply_event(const struct module* self,
                   const struct picotm_event *event,
                   struct picotm_error* error);

void
module_undo_event(const struct module* self,
                  const struct picotm_event *event,
                  struct picotm_error* error);

void
module_update_cc(const struct module* self, bool noundo,
                 struct picotm_error* error);

void
module_clear_cc(const struct module* self, bool noundo,
                struct picotm_error* error);

void
module_finish(const struct module* self, struct picotm_error* error);
