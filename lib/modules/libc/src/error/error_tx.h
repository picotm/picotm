/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>
#include "picotm/picotm-libc.h"

/**
 * \cond impl || libc_impl || libc_impl_error
 * \ingroup libc_impl
 * \ingroup libc_impl_error
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * The data structure |struct error_tx| represents a transaction on the
 * state of the thread's errno variable.
 *
 * A transaction provides error handling for its operations (sans special
 * exceptions). Therefore, the value stored in 'errno' should not change
 * during as transaction. C functions that potentially modify the state of
 * errno shall save the pre-transaction state in |struct error_tx| using
 * the public interface.
 */
struct error_tx {
    unsigned long module;
    enum picotm_libc_error_recovery recovery;
    unsigned long flags;
    int saved_errno;
};

/**
 * Initializes an error transaction.
 */
void
error_tx_init(struct error_tx* self, unsigned long module);

/**
 * Cleans up an error transaction.
 */
void
error_tx_uninit(struct error_tx* self);

/**
 * Sets the transaction's error-recovery strategy.
 */
void
error_tx_set_error_recovery(struct error_tx* self,
                            enum picotm_libc_error_recovery recovery);

/**
 * Returns the transaction's current error-recovery strategy.
 */
enum picotm_libc_error_recovery
error_tx_get_error_recovery(const struct error_tx* self);

/**
 * Returns true if the value of 'errno' has been saved; false otherwise.
 */
bool
error_tx_errno_saved(const struct error_tx* self);

/**
 * Saves the current value from the thread-local errno variable.
 */
void
error_tx_save_errno(struct error_tx* self);

/**
 * Reverts all transaction-local changes.
 */
void
error_tx_undo(struct error_tx* self, struct picotm_error* error);

/**
 * Cleans up a transaction's error state.
 */
void
error_tx_finish(struct error_tx* self, struct picotm_error* error);
