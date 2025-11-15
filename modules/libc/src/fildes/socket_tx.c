/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2019  Thomas Zimmermann
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

#include "socket_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "fcntloptab.h"
#include "socket_tx_ops.h"

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
 * Public interface
 */

void
socket_tx_init(struct socket_tx* self)
{
    assert(self);

    file_tx_init(&self->base, &socket_tx_ops);

    self->sockbuf_tx = NULL;
    self->fcntltab = NULL;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
socket_tx_uninit(struct socket_tx* self)
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
socket_tx_prepare(struct socket_tx* self, struct socket* socket,
                  struct picotm_error* error)
{
    assert(self);

    self->sockbuf_tx = NULL;
    self->fcntltablen = 0;
}

void
socket_tx_release(struct socket_tx* self)
{ }

void
socket_tx_try_rdlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_rdlock_field(socket_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

void
socket_tx_try_wrlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_wrlock_field(socket_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct socket* socket)
{
    enum socket_field field = 0;

    while (beg < end) {
        socket_unlock_field(socket, field, beg);
        ++field;
        ++beg;
    }
}

void
socket_tx_finish(struct socket_tx* self)
{
    /* release reader/writer locks on socket state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    socket_of_base(self->base.file));
}
