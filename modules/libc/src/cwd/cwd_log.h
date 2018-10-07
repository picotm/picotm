/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <stddef.h>
#include "cwd_event.h"

/**
 * \cond impl || libc_impl || libc_impl_cwd
 * \ingroup libc_impl
 * \ingroup libc_impl_cwd
 * \file
 * \endcond
 */

struct picotm_error;

struct cwd_log {

    unsigned long module;

    struct cwd_event* eventtab;
    size_t            eventtablen;
    size_t            eventtabsiz;
};

void
cwd_log_init(struct cwd_log* self, unsigned long module);

void
cwd_log_uninit(struct cwd_log* self);

static inline size_t
cwd_log_length(const struct cwd_log* self)
{
    return self->eventtablen;
}

static inline struct cwd_event*
cwd_log_at(const struct cwd_log* self, size_t i)
{
    return self->eventtab + i;
}

void
cwd_log_clear(struct cwd_log* self);

void
cwd_log_append(struct cwd_log* self, enum cwd_op op, char* alloced,
               struct picotm_error* error);
