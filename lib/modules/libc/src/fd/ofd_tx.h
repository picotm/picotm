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
 * Holds transaction-local state for an open file description.
 */
struct ofd_tx {

    void (*ref)(struct ofd_tx*);
    void (*unref)(struct ofd_tx*);

    enum picotm_libc_file_type type;
};

/**
 * Init transaction-local open-file-description state
 *
 * \param   self    An open file description.
 * \param   type    The open file description's file type.
 * \param   ref     A call-back function to acquire a reference.
 * \param   unref   A call-back function to release a reference.
 */
void
ofd_tx_init(struct ofd_tx* self, enum picotm_libc_file_type type,
            void (*ref)(struct ofd_tx*),
            void (*unref)(struct ofd_tx*));

/**
 * Uninit state
 */
void
ofd_tx_uninit(struct ofd_tx* self);

/**
 * Returns the open file description's file type.
 *
 * \param   self    An ofd structure.
 * \returns The open file description's file type.
 */
enum picotm_libc_file_type
ofd_tx_file_type(const struct ofd_tx* self);

/**
 * Acquire a reference on an open file description.
 *
 * \param   self    An open file description.
 */
void
ofd_tx_ref(struct ofd_tx* self);

/**
 * Release a reference on an open file description.
 *
 * \param   self    An open file description.
 */
void
ofd_tx_unref(struct ofd_tx* self);
