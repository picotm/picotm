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

#include "fifo_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "fcntloptab.h"
#include "fifo_tx_ops.h"

static void
init_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_init(beg);
        ++beg;
    }
}

static void
uninit_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_uninit(beg);
        ++beg;
    }
}

/*
 * Public interfaces
 */

void
fifo_tx_init(struct fifo_tx* self)
{
    assert(self);

    file_tx_init(&self->base, &fifo_tx_ops);

    self->pipebuf_tx = nullptr;

    self->fcntltab = nullptr;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
fifo_tx_uninit(struct fifo_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/*
 * File handling
 */

void
fifo_tx_prepare(struct fifo_tx* self, struct fifo* fifo,
                struct picotm_error* error)
{
    assert(self);

    self->pipebuf_tx = nullptr;
    self->fcntltablen = 0;
}

void
fifo_tx_release(struct fifo_tx* self)
{ }

void
fifo_tx_try_rdlock_field(struct fifo_tx* self, enum fifo_field field,
                         struct picotm_error* error)
{
    assert(self);

    fifo_try_rdlock_field(fifo_of_base(self->base.file), field,
                          self->rwstate + field, error);
}

void
fifo_tx_try_wrlock_field(struct fifo_tx* self, enum fifo_field field,
                         struct picotm_error* error)
{
    assert(self);

    fifo_try_wrlock_field(fifo_of_base(self->base.file), field,
                          self->rwstate + field, error);
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct fifo* fifo)
{
    enum fifo_field field = 0;

    while (beg < end) {
        fifo_unlock_field(fifo, field, beg);
        ++field;
        ++beg;
    }
}

void
fifo_tx_finish(struct fifo_tx* self)
{
    /* release reader/writer locks on FIFO state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    fifo_of_base(self->base.file));
}
