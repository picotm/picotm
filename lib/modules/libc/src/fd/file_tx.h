/* Permission is hereby granted, free of charge, to any person obtaining a
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
 */

#pragma once

#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

/**
 * Holds transaction-local state for a file.
 */
struct file_tx {

    void (*ref)(struct file_tx*);
    void (*unref)(struct file_tx*);

    enum picotm_libc_file_type type;
};

/**
 * \brief Init transaction-local file state.
 * \param   self    A file transaction.
 * \param   type    The open file description's file type.
 * \param   ref     A call-back function to acquire a reference.
 * \param   unref   A call-back function to release a reference.
 */
void
file_tx_init(struct file_tx* self, enum picotm_libc_file_type type,
             void (*ref)(struct file_tx*),
             void (*unref)(struct file_tx*));

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
 * \param   self    A file transaction.
 */
void
file_tx_ref(struct file_tx* self);

/**
 * Release a reference on a file transaction.
 * \param   self    A file transaction.
 */
void
file_tx_unref(struct file_tx* self);
