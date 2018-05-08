/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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

#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-lib-slist.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * Holds transaction-local state for a file.
 */
struct file_tx {

    struct picotm_slist active_list;

    const struct file_tx_ops* ops;
};

static inline struct file_tx*
file_tx_of_slist(struct picotm_slist* item)
{
    return picotm_containerof(item, struct file_tx, active_list);
}

/**
 * \brief Init transaction-local file state.
 * \param   self    A file transaction.
 * \param   ops     The file type's constants and operations.
 */
void
file_tx_init(struct file_tx* self, const struct file_tx_ops* ops);

/**
 * \brief Uninit transaction-local file state.
 * \param   self    A file transaction.
 */
void
file_tx_uninit(struct file_tx* self);

/**
 * \brief Returns the file type.
 * \param   self    A file transaction.
 * \returns The open file description's file type.
 */
enum picotm_libc_file_type
file_tx_file_type(const struct file_tx* self);

/**
 * Acquire a reference on a file transaction.
 * \param       self    A file transaction.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_ref(struct file_tx* self, struct picotm_error* error);

/**
 * Release a reference on a file transaction.
 * \param   self    A file transaction.
 */
void
file_tx_unref(struct file_tx* self);

/*
 * Module interfaces
 */

/**
 * \brief Finishes the transaction-local file state at the end of a
 *        transaction.
 * \param   self    The transaction-local file state.
 */
void
file_tx_finish(struct file_tx* self);
