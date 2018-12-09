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

#include "dir_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "dir_tx_ops.h"
#include "fchmodoptab.h"
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
dir_tx_init(struct dir_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    file_tx_init(&self->base, &dir_tx_ops);

    self->dir = NULL;

    self->wrmode = PICOTM_LIBC_WRITE_THROUGH;

    self->fchmodtab = NULL;
    self->fchmodtablen = 0;

    self->fcntltab = NULL;
    self->fcntltablen = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
dir_tx_uninit(struct dir_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    fchmodoptab_clear(&self->fchmodtab, &self->fchmodtablen);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/*
 * Referencing
 */

void
dir_tx_ref_or_set_up(struct dir_tx* self, struct dir* dir,
                     struct picotm_error* error)
{
    assert(self);
    assert(dir);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on directory */
    dir_ref(dir, error);
    if (picotm_error_is_set(error)) {
        goto err_dir_ref;
    }

    /* setup fields */

    self->dir = dir;

    /* Regular files use write-back mode, but a directory might contain
     * files in write-through mode. to ensure their fsyncs reach disk, we
     * set the directory to write-through mode by default. Fsyncs will be
     * performed during execution and apply phases. */
    self->wrmode = PICOTM_LIBC_WRITE_THROUGH;

    self->fchmodtablen = 0;
    self->fcntltablen = 0;

    return;

err_dir_ref:
    picotm_ref_down(&self->ref);
}

void
dir_tx_ref(struct dir_tx* self, struct picotm_error* error)
{
    picotm_ref_up(&self->ref);
}

void
dir_tx_unref(struct dir_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    dir_unref(self->dir);
    self->dir = NULL;
}

bool
dir_tx_holds_ref(struct dir_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}
