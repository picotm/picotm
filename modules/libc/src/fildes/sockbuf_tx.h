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

#pragma once

#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-libc.h"
#include "picotm/picotm-lib-slist.h"
#include <sys/types.h>
#include "sockbuf.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * Holds transaction-local state for an sockbuf.
 */
struct sockbuf_tx {

    struct picotm_ref16 ref;
    struct picotm_slist list_entry;

    struct sockbuf* sockbuf;

    enum picotm_libc_write_mode wrmode;

    unsigned char* wrbuf;
    size_t         wrbuflen;
    size_t         wrbufsiz;

    struct ioop* wrtab;
    size_t       wrtablen;
    size_t       wrtabsiz;

    /** State of the local reader/writer locks */
    struct picotm_rwstate rwstate[NUMBER_OF_SOCKBUF_FIELDS];
};

static inline struct sockbuf_tx*
sockbuf_tx_of_list_entry(struct picotm_slist *list_entry)
{
    return picotm_containerof(list_entry, struct sockbuf_tx, list_entry);
}

/**
 * Init transaction-local open-file-description state
 */
void
sockbuf_tx_init(struct sockbuf_tx* self);

/**
 * Uninit state
 */
void
sockbuf_tx_uninit(struct sockbuf_tx* self);

/**
 * Acquire a reference on sockbuf state
 */
void
sockbuf_tx_ref_or_set_up(struct sockbuf_tx* self,
                         struct sockbuf* sockbuf,
                         struct picotm_error* error);

/**
 * Acquire a reference
 */
void
sockbuf_tx_ref(struct sockbuf_tx* self);

/**
 * Release reference
 */
void
sockbuf_tx_unref(struct sockbuf_tx* self);

/**
 * Returns true if transaction holds a reference on buffer state.
 */
bool
sockbuf_tx_holds_ref(const struct sockbuf_tx* self);

/**
 * Prepare after acquiring the first reference on the transaction-local
 * sockbuf state.
 */
void
sockbuf_tx_prepare(struct sockbuf_tx* self, struct sockbuf* sockbuf,
                   struct picotm_error* error);

/**
 * Clean up after releasing final reference.
 */
void
sockbuf_tx_release(struct sockbuf_tx* self);

void
sockbuf_tx_try_rdlock_field(struct sockbuf_tx* self,
                            enum sockbuf_field field,
                            struct picotm_error* error);

void
sockbuf_tx_try_wrlock_field(struct sockbuf_tx* self,
                            enum sockbuf_field field,
                            struct picotm_error* error);

int
sockbuf_tx_append_to_writeset(struct sockbuf_tx* self, size_t nbyte,
                              off_t offset, const void* buf,
                              struct picotm_error* error);

void
sockbuf_tx_finish(struct sockbuf_tx* self);
