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

#include "socket_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include <stdlib.h>
#include <string.h>
#include "fcntloptab.h"
#include "iooptab.h"
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

    self->socket = NULL;

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
socket_tx_uninit(struct socket_tx* self)
{
    assert(self);

    fcntloptab_clear(&self->fcntltab, &self->fcntltablen);
    iooptab_clear(&self->wrtab, &self->wrtablen);
    free(self->wrbuf);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

/*
 * File handling
 */

void
socket_tx_acquire_socket(struct socket_tx* self, struct socket* socket,
                         struct picotm_error* error)
{
    assert(self);
    assert(!self->socket);

    /* get reference on socket */
    socket_ref(socket, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    /* setup fields */
    self->socket = socket;
    self->wrmode = PICOTM_LIBC_WRITE_BACK;
    self->fcntltablen = 0;
    self->wrtablen = 0;
    self->wrbuflen = 0;
}

void
socket_tx_release_socket(struct socket_tx* self)
{
    assert(self);
    assert(self->socket);

    socket_unref(self->socket);
    self->socket = NULL;
}

void
socket_tx_try_rdlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_rdlock_field(self->socket, field, self->rwstate + field, error);
}

void
socket_tx_try_wrlock_field(struct socket_tx* self, enum socket_field field,
                           struct picotm_error* error)
{
    assert(self);

    socket_try_wrlock_field(self->socket, field, self->rwstate + field, error);
}

static off_t
append_to_iobuffer(struct socket_tx* self, size_t nbyte, const void* buf,
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
socket_tx_append_to_writeset(struct socket_tx* self, size_t nbyte, off_t offset,
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
                    self->socket);
}
