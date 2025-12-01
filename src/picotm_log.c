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

#include "picotm_log.h"
#include "picotm/picotm-error.h"
#include <assert.h>
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
