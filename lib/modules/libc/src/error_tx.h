/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stdbool.h>

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
    unsigned long flags;
    int saved_errno;
};

/**
 * Initializes an error transaction.
 */
int
error_tx_init(struct error_tx* self, unsigned long module);

/**
 * Cleans up an error transaction.
 */
void
error_tx_uninit(struct error_tx* self);

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
int
error_tx_undo(struct error_tx* self);

/**
 * Cleans up a transaction's error state.
 */
int
error_tx_finish(struct error_tx* self);
