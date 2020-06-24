/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 * Copyright (c) 2020       Thomas Zimmermann <contact@tzimmermann.org>
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

#include "ofd_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include "file_tx.h"
#include "seekop.h"
#include "seekoptab.h"

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

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct ofd* ofd)
{
    enum ofd_field field = 0;

    while (beg < end) {
        ofd_unlock_field(ofd, field, beg);
        ++field;
        ++beg;
    }
}

void
ofd_tx_init(struct ofd_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    picotm_slist_init_item(&self->list_entry);

    self->ofd = NULL;

    self->file_tx = NULL;

    self->seektab = NULL;
    self->seektablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
ofd_tx_uninit(struct ofd_tx* self)
{
    assert(self);

    seekoptab_clear(&self->seektab, &self->seektablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));

    picotm_slist_uninit_item(&self->list_entry);
}

bool
ofd_tx_holds_ref(struct ofd_tx* self)
{
    assert(self);

    return !!self->ofd;
}

int
ofd_tx_cmp_with_id(const struct ofd_tx* self, const struct ofd_id* id)
{
    assert(self);
    assert(self->ofd);

    return ofd_id_cmp(&self->ofd->id, id);
}

/*
 * Reference counting
 */

void
ofd_tx_ref_or_set_up(struct ofd_tx* self, struct ofd* ofd,
                     struct file_tx* file_tx, struct picotm_error* error)
{
    assert(self);
    assert(ofd);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* get references */

    ofd_ref(ofd, error);
    if (picotm_error_is_set(error)) {
        goto err_ofd_ref;
    }

    file_tx_ref(file_tx, error);
    if (picotm_error_is_set(error)) {
        goto err_file_tx_ref;
    }

    /* setup fields */

    self->ofd = ofd;
    self->file_tx = file_tx;

    self->seektablen = 0;

    return;

err_file_tx_ref:
    ofd_unref(ofd);
err_ofd_ref:
    picotm_ref_down(&self->ref);
}

void
ofd_tx_ref(struct ofd_tx* self, struct picotm_error* error)
{
    picotm_ref_up(&self->ref);
}

void
ofd_tx_unref(struct ofd_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    file_tx_unref(self->file_tx);
    ofd_unref(self->ofd);

    self->ofd = NULL;
}

/*
 * Locking
 */

void
ofd_tx_try_rdlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error)
{
    assert(self);

    ofd_try_rdlock_field(self->ofd, field, self->rwstate + field, error);
}

void
ofd_tx_try_wrlock_field(struct ofd_tx* self, enum ofd_field field,
                        struct picotm_error* error)
{
    assert(self);

    ofd_try_wrlock_field(self->ofd, field, self->rwstate + field, error);
}

/*
 * Module interface
 */

void
ofd_tx_finish(struct ofd_tx* self)
{
    assert(self);

    /* release reader/writer locks on open-file-description state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->ofd);
}
