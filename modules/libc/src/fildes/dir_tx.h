/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include <picotm/picotm-lib-ref.h>
#include <picotm/picotm-lib-rwstate.h>
#include <sys/types.h>
#include "dir.h"
#include "file_tx.h"
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct fchmodop;
struct picotm_error;

/**
 * Holds transaction-local state for a directory.
 */
struct dir_tx {

    struct picotm_ref16 ref;

    struct file_tx base;

    struct dir* dir;

    enum picotm_libc_write_mode wrmode;

    struct fchmodop* fchmodtab;
    size_t           fchmodtablen;

    struct fcntlop* fcntltab;
    size_t          fcntltablen;

    /** State of the local locks */
    struct picotm_rwstate rwstate[NUMBER_OF_DIR_FIELDS];
};

/**
 * Initialize transaction-local directory.
 * \param   self    The instance of `struct dir` to initialize.
 */
void
dir_tx_init(struct dir_tx* self);

/**
 * Uninitialize transaction-local directory.
 * \param   self    The instance of `struct dir` to initialize.
 */
void
dir_tx_uninit(struct dir_tx* self);

/**
 * Acquire a reference on the open file description
 */
void
dir_tx_ref_or_set_up(struct dir_tx* self, struct dir* dir,
                     struct picotm_error* error);

/**
 * Acquire a reference on the open file description
 */
void
dir_tx_ref(struct dir_tx* self, struct picotm_error* error);

/**
 * Release reference
 */
void
dir_tx_unref(struct dir_tx* self);

/**
 * Returns true if transactions hold a reference
 */
bool
dir_tx_holds_ref(struct dir_tx* self);
