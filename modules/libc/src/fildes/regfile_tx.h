/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "picotm/picotm-lib-ref.h"
#include "picotm/picotm-lib-rwstate.h"
#include <sys/types.h>
#include "file_tx.h"
#include "picotm/picotm-libc.h"
#include "regfile.h"
#include "rwcountermap.h"

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
struct regfile_tx {

    struct picotm_ref16 ref;

    struct file_tx base;

    struct regfile* regfile;

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

    struct fchmodop* fchmodtab;
    size_t           fchmodtablen;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** Transaction-local file size */
    off_t file_size;

    /** State of the local reader/writer locks. */
    struct picotm_rwstate rwstate[NUMBER_OF_REGFILE_FIELDS];

    /** States of the local region locks */
    struct rwcountermap rwcountermap;

    /** Table of all locked areas */
    struct region*    locktab;
    size_t            locktablen;
    size_t            locktabsiz;
};

/**
 * Init transaction-local open-file-description state
 */
void
regfile_tx_init(struct regfile_tx* self);

/**
 * Uninit state
 */
void
regfile_tx_uninit(struct regfile_tx* self);

/**
 * \brief Returns the transaction-local file size.
 * \param       self    A transaction-local regular-file state.
 * \param       fildes  A reference file descriptor.
 * \param[out]  error   Returns an error to the caller.
 * \returns The transaction-local file size.
 */
off_t
regfile_tx_get_file_size(struct regfile_tx* self, int fildes,
                         struct picotm_error* error);

/**
 * \brief Sets the transaction-local file size.
 * \param       self    A transaction-local regular-file state.
 * \param       size    The new file size.
 * \param[out]  error   Returns an error to the caller.
 * \returns The transaction-local file size.
 */
void
regfile_tx_set_file_size(struct regfile_tx* self, off_t size,
                         struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
regfile_tx_ref_or_set_up(struct regfile_tx* self, struct regfile* regfile,
                         struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
regfile_tx_ref(struct regfile_tx* self, struct picotm_error* error);

/**
 * Release reference
 */
void
regfile_tx_unref(struct regfile_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
regfile_tx_holds_ref(struct regfile_tx* self);
