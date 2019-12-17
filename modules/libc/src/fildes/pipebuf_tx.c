/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "pipebuf_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include <stdlib.h>
#include <string.h>
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
pipebuf_tx_init(struct pipebuf_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);
    picotm_slist_init_item(&self->list_entry);

    self->pipebuf = NULL;

    self->wrmode = PICOTM_LIBC_WRITE_BACK;

    self->wrbuf = NULL;
    self->wrbuflen = 0;
    self->wrbufsiz = 0;

    self->wrtab = NULL;
    self->wrtablen = 0;
    self->wrtabsiz = 0;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
pipebuf_tx_uninit(struct pipebuf_tx* self)
{
    assert(self);
    assert(!self->pipebuf);

    iooptab_clear(&self->wrtab, &self->wrtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));

    picotm_slist_uninit_item(&self->list_entry);
}

/*
 * Referencing
 */

void
pipebuf_tx_ref_or_set_up(struct pipebuf_tx* self,
                         struct pipebuf* pipebuf,
                         struct picotm_error* error)
{
    assert(self);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on shared file state */
    pipebuf_ref(pipebuf, error);
    if (picotm_error_is_set(error)) {
        goto err_pipebuf_ref;
    }

    self->pipebuf = pipebuf;
    self->wrmode = PICOTM_LIBC_WRITE_BACK;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    return;

err_pipebuf_ref:
    picotm_ref_down(&self->ref);
}

void
pipebuf_tx_ref(struct pipebuf_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
pipebuf_tx_unref(struct pipebuf_tx* self)
{
    assert(self);
    assert(self->pipebuf);

    bool final_unref = picotm_ref_down(&self->ref);
    if (!final_unref) {
        return;
    }

    pipebuf_unref(self->pipebuf);
    self->pipebuf = NULL;
}

bool
pipebuf_tx_holds_ref(const struct pipebuf_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

/*
 * File handling
 */

void
pipebuf_tx_try_rdlock_field(struct pipebuf_tx* self,
                            enum pipebuf_field field,
                            struct picotm_error* error)
{
    assert(self);

    pipebuf_try_rdlock_field(self->pipebuf, field,
                             self->rwstate + field, error);
}

void
pipebuf_tx_try_wrlock_field(struct pipebuf_tx* self,
                            enum pipebuf_field field,
                            struct picotm_error* error)
{
    assert(self);

    pipebuf_try_wrlock_field(self->pipebuf, field,
                             self->rwstate + field, error);
}

static off_t
append_to_iobuffer(struct pipebuf_tx* self, size_t nbyte, const void* buf,
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
pipebuf_tx_append_to_writeset(struct pipebuf_tx* self, size_t nbyte,
                              off_t offset, const void* buf,
                              struct picotm_error* error)
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
                struct pipebuf* pipebuf)
{
    enum pipebuf_field field = 0;

    while (beg < end) {
        pipebuf_unlock_field(pipebuf, field, beg);
        ++field;
        ++beg;
    }
}

/*
 * Module interfaces
 */

void
pipebuf_tx_finish(struct pipebuf_tx* self)
{
    /* release reader/writer locks on FIFO state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->pipebuf);
}
