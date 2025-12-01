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

#include "rwcounter.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-rwlock.h"
#include <assert.h>
#include <stdbool.h>

enum {
    RWCOUNTER_WRITTEN = 0x8000000, /* flags write-lock counters */
    RWCOUNTER_BITS    = 0x7ffffff  /* counter bits */
};

void
rwcounter_init(struct rwcounter* self)
{
    assert(self);

    self->state = 0;
}

void
rwcounter_rdlock(struct rwcounter* self, struct picotm_rwlock* rwlock,
                 struct picotm_error* error)
{
    assert(self);

    if ( !(self->state & RWCOUNTER_BITS) ) {

        /* not yet read-locked */

        picotm_rwlock_try_rdlock(rwlock, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    self->state = (self->state & RWCOUNTER_WRITTEN) |
                 ((self->state & RWCOUNTER_BITS) + 1);
}

void
rwcounter_wrlock(struct rwcounter* self, struct picotm_rwlock* rwlock,
                 struct picotm_error* error)
{
    assert(self);

    if ( !(self->state & RWCOUNTER_WRITTEN) ) {

        /* not yet write-locked */

        bool upgrade = !!(self->state & RWCOUNTER_BITS);

        picotm_rwlock_try_wrlock(rwlock, upgrade, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }

    self->state = RWCOUNTER_WRITTEN | ((self->state & RWCOUNTER_BITS) + 1);
}

void
rwcounter_unlock(struct rwcounter* self, struct picotm_rwlock* rwlock)
{
    assert(self);
    assert(self->state & RWCOUNTER_BITS);

    self->state = (self->state & RWCOUNTER_WRITTEN) |
                 ((self->state & RWCOUNTER_BITS) - 1);

    if (self->state & RWCOUNTER_BITS) {
        return;
    }

    /* last lock of this transaction; unlock */
    picotm_rwlock_unlock(rwlock);
    self->state = 0;
}
