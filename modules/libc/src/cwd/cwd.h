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

#include <picotm/picotm-lib-rwlock.h>

/**
 * \cond impl || libc_impl || libc_impl_cwd
 * \ingroup libc_impl
 * \ingroup libc_impl_cwd
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_rwstate;

/**
 * Enumerates the fields of `struct cwd`.
 */
enum cwd_field {
    CWD_FIELD_STRING,
    NUMBER_OF_CWD_FIELDS
};

/**
 * Represents the global state of the current working directory.
 */
struct cwd {
    struct picotm_rwlock    rwlock[NUMBER_OF_CWD_FIELDS];
};

/**
 * Initialized a CWD structure
 *
 * \param   self    The CWD structure.
 */
void
cwd_init(struct cwd* self);

/**
 * Uninitialized a CWD structure
 *
 * \param   self    The CWD structure.
 */
void
cwd_uninit(struct cwd* self);

/**
 * Tries to acquire a reader lock on the current working directory.
 *
 * \param       self    The CWD structure.
 * \param       field   The reader lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
cwd_try_rdlock_field(struct cwd* self, enum cwd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * Tries to acquire a writer lock on the current working directory.
 *
 * \param       self    The CWD structure.
 * \param       field   The writer lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
cwd_try_wrlock_field(struct cwd* self, enum cwd_field field,
                     struct picotm_rwstate* rwstate,
                     struct picotm_error* error);

/**
 * Releases a reader/writer lock on the current working directory.
 *
 * \param   self    The CWD structure.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader-writer state.
 */
void
cwd_unlock_field(struct cwd* self, enum cwd_field field,
                 struct picotm_rwstate* rwstate);
