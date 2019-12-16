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
#include <sys/types.h>
#include "file_tx.h"
#include "picotm/picotm-libc.h"
#include "rwcountermap.h"
#include "seekbuf.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * Holds transaction-local reads and writes for a regular file.
 */
struct seekbuf_tx {

    struct picotm_ref16 ref;
    struct picotm_slist list_entry;

    struct seekbuf *seekbuf;

    enum picotm_libc_write_mode wrmode;

    unsigned char* wrbuf;
    size_t         wrbuflen;
    size_t         wrbufsiz;

    struct ioop* wrtab;
    size_t       wrtablen;
    size_t       wrtabsiz;

    struct ioop* rdtab;
    size_t       rdtablen;
    size_t       rdtabsiz;

    /** Transaction-local buffer size */
    off_t file_size;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_SEEKBUF_FIELDS];

    /** States of the local region locks */
    struct rwcountermap rwcountermap;

    /** Table of all locked areas */
    struct region*    locktab;
    size_t            locktablen;
    size_t            locktabsiz;
};

static inline struct seekbuf_tx*
seekbuf_tx_of_list_entry(struct picotm_slist *list_entry)
{
    return picotm_containerof(list_entry, struct seekbuf_tx, list_entry);
}

/**
 * Init transaction-local open-file-description state
 */
void
seekbuf_tx_init(struct seekbuf_tx* self);

/**
 * Uninit state
 */
void
seekbuf_tx_uninit(struct seekbuf_tx* self);

/**
 * Acquire a reference on seekbuf state
 */
void
seekbuf_tx_ref_or_set_up(struct seekbuf_tx* self, struct seekbuf *seekbuf,
                         struct picotm_error* error);

/**
 * Acquire a reference
 */
void
seekbuf_tx_ref(struct seekbuf_tx* self);

/**
 * Release reference
 */
void
seekbuf_tx_unref(struct seekbuf_tx* self);

/**
 * Returns true if transaction holds a reference on buffer state.
 */
bool
seekbuf_tx_holds_ref(const struct seekbuf_tx* self);

/**
 * \brief Returns the transaction-local file size.
 * \param       self    A transaction-local regular-file state.
 * \param       fildes  A reference file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns The transaction-local file size.
 */
off_t
seekbuf_tx_get_file_size(struct seekbuf_tx* self, int fildes,
                         struct picotm_error* error);

/**
 * \brief Sets the transaction-local file size.
 * \param       self    A transaction-local regular-file state.
 * \param       size    The new file size.
 * \param[out]  error   Returns an error to the caller.
 * \returns The transaction-local file size.
 */
void
seekbuf_tx_set_file_size(struct seekbuf_tx* self, off_t size,
                         struct picotm_error* error);

/**
 * Prepare after acquiring the first reference on the transaction-local
 * regular-file state.
 */
void
seekbuf_tx_prepare(struct seekbuf_tx* self, struct seekbuf* seekbuf,
                   struct picotm_error* error);

/**
 * Clean up after releasing reference.
 */
void
seekbuf_tx_release(struct seekbuf_tx* self);

void
seekbuf_tx_try_rdlock_field(struct seekbuf_tx* self, enum seekbuf_field field,
                            struct picotm_error* error);

void
seekbuf_tx_try_wrlock_field(struct seekbuf_tx* self, enum seekbuf_field field,
                            struct picotm_error* error);

int
seekbuf_tx_try_rdlock_region(struct seekbuf_tx* self, size_t nbyte,
                             off_t offset, struct picotm_error* error);

int
seekbuf_tx_try_wrlock_region(struct seekbuf_tx* self, size_t nbyte,
                             off_t offset, struct picotm_error* error);

int
seekbuf_tx_append_to_writeset(struct seekbuf_tx* self, size_t nbyte, off_t offset,
                              const void* buf, struct picotm_error* error);

int
seekbuf_tx_append_to_readset(struct seekbuf_tx* self, size_t nbyte, off_t offset,
                             const void* buf, struct picotm_error* error);

void
seekbuf_tx_finish(struct seekbuf_tx* self);
