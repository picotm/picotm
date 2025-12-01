/*
 * picotm - A system-level transaction manager
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

#include "locale_log.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include <string.h>

void
locale_log_init(struct locale_log* self, unsigned long module)
{
    assert(self);

    self->module = module;
    self->eventtab = nullptr;
    self->eventtablen = 0;
    self->eventtabsiz = 0;
}

void
locale_log_uninit(struct locale_log* self)
{
    assert(self);

    picotm_tabfree(self->eventtab);
}

void
locale_log_clear(struct locale_log* self)
{
    assert(self);

    self->eventtablen = 0;
}

static struct locale_event*
locale_log_end_alloc(struct locale_log* self,
                     struct picotm_error* error)
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
        return nullptr;
    }
    self->eventtab = tmp;
    self->eventtabsiz = neweventtabsiz;

out:
    return locale_log_at(self, self->eventtablen++);
}

void
locale_log_append(struct locale_log* self, enum locale_op op,
                  struct locale_event* arg, struct picotm_error* error)
{
    assert(self);

    struct locale_event* event = locale_log_end_alloc(self, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    memcpy(event, arg, sizeof(*event));

    picotm_append_event(self->module, op, (uintptr_t)(event - self->eventtab),
                        error);
    if (picotm_error_is_set(error)) {
        return;
    }
}
