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

#include "chrdev_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include <stdlib.h>
#include <string.h>
#include "chrdev_tx_ops.h"
#include "fcntloptab.h"
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
 * Public interface
 */

void
chrdev_tx_init(struct chrdev_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);

    file_tx_init(&self->base, &chrdev_tx_ops);

    self->chrdev = NULL;

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
chrdev_tx_uninit(struct chrdev_tx* self)
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
chrdev_tx_ref_or_set_up(struct chrdev_tx* self, struct chrdev* chrdev,
                        struct picotm_error* error)
{
    assert(self);
    assert(chrdev);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on character device */
    chrdev_ref(chrdev, error);
    if (picotm_error_is_set(error)) {
        goto err_chrdev_ref;
    }

    /* setup fields */

    self->chrdev = chrdev;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->fcntltablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    return;

err_chrdev_ref:
    picotm_ref_down(&self->ref);
}

void
chrdev_tx_ref(struct chrdev_tx* self, struct picotm_error* error)
{
    picotm_ref_up(&self->ref);
}

void
chrdev_tx_unref(struct chrdev_tx* self)
{
    assert(self);

    bool final_ref = picotm_ref_down(&self->ref);
    if (!final_ref) {
        return;
    }

    chrdev_unref(self->chrdev);
    self->chrdev = NULL;
}

bool
chrdev_tx_holds_ref(struct chrdev_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

void
chrdev_tx_try_rdlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_rdlock_field(self->chrdev, field, self->rwstate + field, error);
}

void
chrdev_tx_try_wrlock_field(struct chrdev_tx* self, enum chrdev_field field,
                           struct picotm_error* error)
{
    assert(self);

    chrdev_try_wrlock_field(self->chrdev, field, self->rwstate + field, error);
}

static off_t
append_to_iobuffer(struct chrdev_tx* self, size_t nbyte, const void* buf,
                   struct picotm_error* error)
{
    off_t bufoffset;

    assert(self);

    bufoffset = self->wrbuflen;

    if (nbyte && buf) {

        /* resize */
        void* tmp = picotm_tabresize(self->wrbuf,
                                     self->wrbuflen,
                                     self->wrbuflen+nbyte,
                                     sizeof(self->wrbuf[0]),
                                     error);
        if (picotm_error_is_set(error)) {
            return (off_t)-1;
        }
        self->wrbuf = tmp;

        /* append */
        memcpy(self->wrbuf+self->wrbuflen, buf, nbyte);
        self->wrbuflen += nbyte;
    }

    return bufoffset;
}

int
chrdev_tx_append_to_writeset(struct chrdev_tx* self, size_t nbyte, off_t offset,
                             const void* buf, struct picotm_error* error)
{
    assert(self);

    off_t bufoffset = append_to_iobuffer(self, nbyte, buf, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    unsigned long res = iooptab_append(&self->wrtab,
                                       &self->wrtablen,
                                       &self->wrtabsiz,
                                       nbyte, offset, bufoffset,
                                       error);
    if (picotm_error_is_set(error)) {
        return -1;
    }
    return res;
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
                    self->chrdev);
}
