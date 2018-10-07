/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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
