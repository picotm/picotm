/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
