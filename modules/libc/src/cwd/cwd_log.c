/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "cwd_log.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>

void
cwd_log_init(struct cwd_log* self, unsigned long module)
{
    assert(self);

    self->module = module;
    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;
}

void
cwd_log_uninit(struct cwd_log* self)
{
    assert(self);

    picotm_tabfree(self->eventtab);
}

void
cwd_log_clear(struct cwd_log* self)
{
    assert(self);

    self->eventtablen = 0;
}

static struct cwd_event*
cwd_log_end_alloc(struct cwd_log* self, struct picotm_error* error)
{
    assert(self);

    if (self->eventtablen < self->eventtabsiz) {
        goto out;
    }

    size_t neweventtabsiz = self->eventtabsiz + 1;

    void* tmp = picotm_tabresize(self->eventtab, self->eventtabsiz,
                                 neweventtabsiz, sizeof(self->eventtab[0]),
                                 error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    self->eventtab = tmp;
    self->eventtabsiz = neweventtabsiz;

out:
    return cwd_log_at(self, self->eventtablen++);
}

void
cwd_log_append(struct cwd_log* self, enum cwd_op op, char* alloced,
               struct picotm_error* error)
{
    assert(self);

    struct cwd_event* event = cwd_log_end_alloc(self, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    event->alloced = alloced;

    picotm_append_event(self->module, op, (uintptr_t)(event - self->eventtab),
                        error);
    if (picotm_error_is_set(error)) {
        return;
    }
}
