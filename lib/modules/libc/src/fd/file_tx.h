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

#include <sys/queue.h>

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

    SLIST_ENTRY(file_tx) active_list;

    const struct file_tx_ops* ops;
};

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

/*
 * Module interfaces
 */

/**
 * Locks the transaction-local file state before commit or validation.
 * \param   self        The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_lock(struct file_tx* self, struct picotm_error* error);

/**
 * Unlocks the transaction-local file state after commit or validation.
 * \param   self        The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_unlock(struct file_tx* self, struct picotm_error* error);

/**
 * Validates the transaction-local file state.
 * \param       self    The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_validate(struct file_tx* self, struct picotm_error* error);

/**
 * \brief Updates the concurrency control on transaction-local file state
 *        after a successful apply.
 * \param       self    The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_update_cc(struct file_tx* self, struct picotm_error* error);

/**
 * \brief Clears the concurrency control on transaction-local file state
 *        after a successful apply.
 * \param       self    The transaction-local file state.
 * \param[out]  error   Returns an error to the caller.
 */
void
file_tx_clear_cc(struct file_tx* self, struct picotm_error* error);
