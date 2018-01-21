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

#include "picotm_log.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <string.h>
#include "picotm_event.h"
#include "table.h"

void
picotm_log_init(struct picotm_log* self)
{
    assert(self);

    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;
}

void
picotm_log_uninit(struct picotm_log* self)
{
    assert(self);

    tabfree(self->eventtab);
}

struct picotm_event*
picotm_log_begin(struct picotm_log* self)
{
    assert(self);

    return self->eventtab;
}

const struct picotm_event*
picotm_log_end(struct picotm_log* self)
{
    assert(self);

    return self->eventtab + self->eventtablen;
}

void
picotm_log_clear(struct picotm_log* self)
{
    self->eventtablen = 0;
}

static struct picotm_event*
picotm_log_end_alloc(struct picotm_log* self, struct picotm_error* error)
{
    assert(self);

    size_t neweventtabsiz = self->eventtablen + 1;

    if (neweventtabsiz <= self->eventtabsiz) {
        goto out;
    }

    void* tmp = tabresize(self->eventtab,
                          self->eventtabsiz, neweventtabsiz,
                          sizeof(self->eventtab[0]),
                          error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    self->eventtab = tmp;
    self->eventtabsiz = neweventtabsiz;

out:
    return (struct picotm_event*)picotm_log_end(self);
}

static void
picotm_log_end_seal(struct picotm_log* self, const struct picotm_event* event)
{
    assert(self);
    assert(event == picotm_log_end(self));

    ++self->eventtablen;
}

void
picotm_log_append(struct picotm_log* self, const struct picotm_event* event,
           struct picotm_error* error)
{
    assert(self);
    assert(event);

    struct picotm_event* end = picotm_log_end_alloc(self, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    memcpy(end, event, sizeof(*event));

    picotm_log_end_seal(self, end);
}
