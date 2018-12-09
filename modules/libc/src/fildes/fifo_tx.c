/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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
#include <stdlib.h>
#include "fcntloptab.h"
#include "fifo_tx_ops.h"
#include "iooptab.h"

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

    picotm_ref_init(&self->ref, 0);

    file_tx_init(&self->base, &fifo_tx_ops);

    self->fifo = NULL;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->wrbuf = NULL;
    self->wrbuflen = 0;
    self->wrbufsiz = 0;

    self->wrtab = NULL;
    self->wrtablen = 0;
    self->wrtabsiz = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
fifo_tx_uninit(struct fifo_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    iooptab_clear(&self->wrtab, &self->wrtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/* Referencing
 */

void
fifo_tx_ref_or_set_up(struct fifo_tx* self, struct fifo* fifo,
                      struct picotm_error* error)
{
    assert(self);
    assert(fifo);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get reference on FIFO */
    fifo_ref(fifo, error);
    if (picotm_error_is_set(error)) {
        goto err_fifo_ref;
    }

    /* setup fields */

    self->fifo = fifo;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->fcntltablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    return;

err_fifo_ref:
    picotm_ref_down(&self->ref);
}

void
fifo_tx_ref(struct fifo_tx* self, struct picotm_error* error)
{
    picotm_ref_up(&self->ref);
}

void
fifo_tx_unref(struct fifo_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    fifo_unref(self->fifo);
    self->fifo = NULL;
}

bool
fifo_tx_holds_ref(struct fifo_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

