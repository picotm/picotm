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

#include "sockbuf_tx.h"
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
sockbuf_tx_init(struct sockbuf_tx* self)
{
    assert(self);

    picotm_ref_init(&self->ref, 0);
    picotm_slist_init_item(&self->list_entry);

    self->sockbuf = NULL;

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
sockbuf_tx_uninit(struct sockbuf_tx* self)
{
    assert(self);
    assert(!self->sockbuf);

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
sockbuf_tx_ref_or_set_up(struct sockbuf_tx* self,
                         struct sockbuf* sockbuf,
                         struct picotm_error* error)
{
    assert(self);

    bool first_ref = picotm_ref_up(&self->ref);
    if (!first_ref) {
        return;
    }

    /* acquire reference on shared file state */
    sockbuf_ref(sockbuf, error);
    if (picotm_error_is_set(error)) {
        goto err_sockbuf_ref;
    }

    self->sockbuf = sockbuf;
    self->wrmode = PICOTM_LIBC_WRITE_BACK;
    self->wrtablen = 0;
    self->wrbuflen = 0;

    return;

err_sockbuf_ref:
    picotm_ref_down(&self->ref);
}

void
sockbuf_tx_ref(struct sockbuf_tx* self)
{
    picotm_ref_up(&self->ref);
}

void
sockbuf_tx_unref(struct sockbuf_tx* self)
{
    assert(self);
    assert(self->sockbuf);

    bool final_unref = picotm_ref_down(&self->ref);
    if (!final_unref) {
        return;
    }

    sockbuf_unref(self->sockbuf);
    self->sockbuf = NULL;
}

bool
sockbuf_tx_holds_ref(const struct sockbuf_tx* self)
{
    assert(self);

    return picotm_ref_count(&self->ref) > 0;
}

/*
 * File handling
 */

void
sockbuf_tx_try_rdlock_field(struct sockbuf_tx* self,
                            enum sockbuf_field field,
                            struct picotm_error* error)
{
    assert(self);

    sockbuf_try_rdlock_field(self->sockbuf, field,
                             self->rwstate + field, error);
}

void
sockbuf_tx_try_wrlock_field(struct sockbuf_tx* self,
                            enum sockbuf_field field,
                            struct picotm_error* error)
{
    assert(self);

    sockbuf_try_wrlock_field(self->sockbuf, field,
                             self->rwstate + field, error);
}

static off_t
append_to_iobuffer(struct sockbuf_tx* self, size_t nbyte, const void* buf,
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
sockbuf_tx_append_to_writeset(struct sockbuf_tx* self, size_t nbyte,
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
                struct sockbuf* sockbuf)
{
    enum sockbuf_field field = 0;

    while (beg < end) {
        sockbuf_unlock_field(sockbuf, field, beg);
        ++field;
        ++beg;
    }
}

/*
 * Module interfaces
 */

void
sockbuf_tx_finish(struct sockbuf_tx* self)
{
    /* release reader/writer locks on FIFO state */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->sockbuf);
}
