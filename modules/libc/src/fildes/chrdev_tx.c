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

#include "chrdev_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "chrdev_tx_ops.h"
#include "fcntloptab.h"

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
chrdev_tx_init(struct chrdev_tx* self)
{
    assert(self);

    file_tx_init(&self->base, &chrdev_tx_ops);

    self->seekbuf_tx = NULL;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
chrdev_tx_uninit(struct chrdev_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/* Referencing
 */

void
chrdev_tx_prepare(struct chrdev_tx* self, struct chrdev* chrdev,
                  struct picotm_error* error)
{
    assert(self);

    self->seekbuf_tx = NULL;
    self->fcntltablen = 0;
}

void
chrdev_tx_release(struct chrdev_tx* self)
{ }

void
chrdev_tx_try_rdlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_rdlock_field(chrdev_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

void
chrdev_tx_try_wrlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_wrlock_field(chrdev_of_base(self->base.file), field,
                            self->rwstate + field, error);
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct chrdev* chrdev)
{
    enum chrdev_field field = 0;

    while (beg < end) {
        chrdev_unlock_field(chrdev, field, beg);
        ++field;
        ++beg;
    }
}

void
chrdev_tx_finish(struct chrdev_tx* self)
{
    /* release reader/writer locks on character-device state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    chrdev_of_base(self->base.file));
}
