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

#include "fdtab_tx.h"
#include "picotm/picotm-error.h"
#include <assert.h>
#include "fildes.h"

void
fdtab_tx_init(struct fdtab_tx* self, struct fildes* fildes)
{
    assert(self);

    picotm_rwstate_init(&self->rwstate);

    self->fildes = fildes;
}

void
fdtab_tx_uninit(struct fdtab_tx* self)
{
    assert(self);

    picotm_rwstate_uninit(&self->rwstate);
}

void
fdtab_tx_try_rdlock(struct fdtab_tx* self, struct picotm_error* error)
{
    assert(self);

    fildes_try_rdlock_fdtab(self->fildes, &self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
fdtab_tx_try_wrlock(struct fdtab_tx* self, struct picotm_error* error)
{
    assert(self);

    fildes_try_wrlock_fdtab(self->fildes, &self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

struct fd*
fdtab_tx_ref_fildes(struct fdtab_tx* self, int fildes,
                    struct picotm_error* error)
{
    assert(self);

    fildes_try_rdlock_fdtab(self->fildes, &self->rwstate, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct fd* fd = fildes_ref_fd(self->fildes, fildes, &self->rwstate,
                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}

/*
 * Module Interface
 */

void
fdtab_tx_finish(struct fdtab_tx* self)
{
    assert(self);

    fildes_unlock_fdtab(self->fildes, &self->rwstate);
}
