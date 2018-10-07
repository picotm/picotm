/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/picotm-libc.h"
#include <signal.h>
#include <stdbool.h>

/**
 * \cond impl || libc_impl || libc_impl_signal
 * \ingroup libc_impl
 * \ingroup libc_impl_signal
 * \file
 * \endcond
 */

struct picotm_error;

/**
 * The data structure |struct signal_tx| represents a transaction on the
 * state of the thread's errno variable.
 *
 * A transaction provides error handling for its operations (sans special
 * exceptions). Therefore, the value stored in 'errno' should not change
 * during as transaction. C functions that potentially modify the state of
 * errno shall save the pre-transaction state in |struct error_tx| using
 * the public interface.
 */
struct signal_tx {
    unsigned long module;

    volatile sig_atomic_t tx_is_running;

    /* 0: non-tx, 1: recoverable, 2: non-recoverable */
    volatile sig_atomic_t signal_state[NSIG];
};

/**
 * Initializes an error transaction.
 * \param[out]  self    The signal-handler transaction.
 * \param       module  The signal module's index.
 */
void
signal_tx_init(struct signal_tx* self, unsigned long module);

/**
 * Cleans up an error transaction.
 * \param   self    The signal-handler transaction.
 */
void
signal_tx_uninit(struct signal_tx* self);

/**
 * Enables signal handling for a specific signal.
 * \param       self            The signal-handler transaction.
 * \param       signum          The number of the signal to handle.
 * \param       is_recoverable  Send a recoverable error to the transaction.
 * \param[out]  error           Returns an error to the caller.
 */
void
signal_tx_add_signal(struct signal_tx* self, int signum, bool is_recoverable,
                     struct picotm_error* error);

/**
 * Disables signal handling for a specific signal.
 * \param   self    The signal-handler transaction.
 * \param   signum  The number of the signal to remove.
 */
void
signal_tx_remove_signal(struct signal_tx* self, int signum);

/**
 * Disables signal handling for all enabled signals.
 * \param   self    The signal-handler transaction.
 */
void
signal_tx_clear_signals(struct signal_tx* self);

void
signal_tx_recover_from_signal(struct signal_tx* self, const siginfo_t* info);

/**
 * Sets up a new transaction's signal handling.
 */
void
signal_tx_begin(struct signal_tx* self, struct picotm_error* error);

/**
 * Cleans up a transaction's signal handling.
 */
void
signal_tx_finish(struct signal_tx* self, struct picotm_error* error);
