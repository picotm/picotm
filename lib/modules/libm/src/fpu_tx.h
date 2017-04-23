/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <fenv.h>
#include <stdbool.h>

/**
 * The data structure |struct fpu_tx| represents a transaction on the
 * state of the thread's FPU state.
 *
 * Transactional C functions potentially modify the state of the
 * thread's floating-point environment or status flags. Before
 * executing such a change, a function shall save the pre-transaction
 * state in |struct fpu_tx| using the public interface.
 */
struct fpu_tx {
    unsigned long module;

    unsigned long flags;

    fenv_t    saved_fenv;
    fexcept_t saved_fexcept;
};

/**
 * Initializes an FPU transaction.
 */
int
fpu_tx_init(struct fpu_tx* self, unsigned long module);

/**
 * Cleans up an FPU transaction.
 */
void
fpu_tx_uninit(struct fpu_tx* self);

/**
 * Returns true if the floating-point environment has has been saved;
 * false otherwise.
 */
bool
fpu_tx_fenv_saved(const struct fpu_tx* self);

/**
 * Returns true if the floating-point status flags have has been saved;
 * false otherwise.
 */
bool
fpu_tx_fexcept_saved(const struct fpu_tx* self);

/**
 * Saves the current thread-local floating-point environment.
 */
int
fpu_tx_save_fenv(struct fpu_tx* self);

/**
 * Saves the current thread-local floating-point status flags.
 */
int
fpu_tx_save_fexcept(struct fpu_tx* self);

/**
 * Reverts all transaction-local changes.
 */
int
fpu_tx_undo(struct fpu_tx* self);

/**
 * Cleans up a transaction's FPU state.
 */
int
fpu_tx_finish(struct fpu_tx* self);
