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
