/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "fildes_log.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <assert.h>

void
fildes_log_init(struct fildes_log* self, unsigned long module)
{
    assert(self);

    self->module = module;
    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;
}

void
fildes_log_uninit(struct fildes_log* self)
{
    assert(self);

    picotm_tabfree(self->eventtab);
}

void
fildes_log_clear(struct fildes_log* self)
{
    assert(self);

    self->eventtablen = 0;
}

static struct fildes_event*
fildes_log_end_alloc(struct fildes_log* self, struct picotm_error* error)
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
    return fildes_log_at(self, self->eventtablen++);
}

void
fildes_log_append(struct fildes_log* self, enum fildes_op op, int fildes,
                  int cookie, struct picotm_error* error)
{
    assert(self);

    struct fildes_event* event = fildes_log_end_alloc(self, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    event->fildes = fildes;
    event->cookie = cookie;

    picotm_append_event(self->module, op, (uintptr_t)(event - self->eventtab),
                        error);
    if (picotm_error_is_set(error)) {
        return;
    }
}
