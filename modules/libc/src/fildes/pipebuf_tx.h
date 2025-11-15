/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2019-2020  Thomas Zimmermann
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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-lib-slist.h"
#include "picotm/picotm-libc.h"
#include <sys/types.h>
#include "pipebuf.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * Holds transaction-local state for an pipebuf.
 */
struct pipebuf_tx {

    struct picotm_ref16 ref;
    struct picotm_slist list_entry;

    struct pipebuf* pipebuf;

    enum picotm_libc_write_mode wrmode;

    unsigned char* wrbuf;
    size_t         wrbuflen;
    size_t         wrbufsiz;

    struct ioop* wrtab;
    size_t       wrtablen;
    size_t       wrtabsiz;

    /** State of the local reader/writer locks */
    struct picotm_rwstate rwstate[NUMBER_OF_PIPEBUF_FIELDS];
};

static inline struct pipebuf_tx*
pipebuf_tx_of_list_entry(struct picotm_slist *list_entry)
{
    return picotm_containerof(list_entry, struct pipebuf_tx, list_entry);
}

/**
 * Init transaction-local open-file-description state
 */
void
pipebuf_tx_init(struct pipebuf_tx* self);

/**
 * Uninit state
 */
void
pipebuf_tx_uninit(struct pipebuf_tx* self);

/**
 * Acquire a reference on pipebuf state
 */
void
pipebuf_tx_ref_or_set_up(struct pipebuf_tx* self,
                         struct pipebuf* pipebuf,
                         struct picotm_error* error);

/**
 * Acquire a reference
 */
void
pipebuf_tx_ref(struct pipebuf_tx* self);

/**
 * Release reference
 */
void
pipebuf_tx_unref(struct pipebuf_tx* self);

/**
 * Returns true if transaction holds a reference on buffer state.
 */
bool
pipebuf_tx_holds_ref(const struct pipebuf_tx* self);

/**
 * Prepare after acquiring the first reference on the transaction-local
 * pipebuf state.
 */
void
pipebuf_tx_prepare(struct pipebuf_tx* self, struct pipebuf* pipebuf,
                   struct picotm_error* error);

/**
 * Clean up after releasing final reference.
 */
void
pipebuf_tx_release(struct pipebuf_tx* self);

void
pipebuf_tx_try_rdlock_field(struct pipebuf_tx* self,
                            enum pipebuf_field field,
                            struct picotm_error* error);

void
pipebuf_tx_try_wrlock_field(struct pipebuf_tx* self,
                            enum pipebuf_field field,
                            struct picotm_error* error);

int
pipebuf_tx_append_to_writeset(struct pipebuf_tx* self, size_t nbyte,
                              off_t offset, const void* buf,
                              struct picotm_error* error);

void
pipebuf_tx_finish(struct pipebuf_tx* self);
