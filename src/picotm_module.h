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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_module_ops;

struct picotm_module {
    const struct picotm_module_ops* ops;
    void* data;
};

void
picotm_module_init(struct picotm_module* self,
                   const struct picotm_module_ops* ops,
                   void* data);

void
picotm_module_uninit(struct picotm_module* self);

void *
picotm_module_get_data(const struct picotm_module* self);

void
picotm_module_begin(const struct picotm_module* self,
                    struct picotm_error* error);

void
picotm_module_lock(const struct picotm_module* self,
                   struct picotm_error* error);

void
picotm_module_unlock(const struct picotm_module* self,
                     struct picotm_error* error);

void
picotm_module_validate(const struct picotm_module* self, bool noundo,
                       struct picotm_error* error);

void
picotm_module_apply(const struct picotm_module* self,
                    struct picotm_error* error);

void
picotm_module_undo(const struct picotm_module* self,
                   struct picotm_error* error);

void
picotm_module_apply_event(const struct picotm_module* self,
                          uint16_t head, uintptr_t tail,
                          struct picotm_error* error);

void
picotm_module_undo_event(const struct picotm_module* self,
                         uint16_t head, uintptr_t tail,
                         struct picotm_error* error);

void
picotm_module_update_cc(const struct picotm_module* self, bool noundo,
                        struct picotm_error* error);

void
picotm_module_clear_cc(const struct picotm_module* self, bool noundo,
                       struct picotm_error* error);

void
picotm_module_finish(const struct picotm_module* self,
                     struct picotm_error* error);
