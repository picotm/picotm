/*
 * picotm - A system-level transaction manager
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

#include <fenv.h>
#include <stdbool.h>

/**
 * \cond impl || libm_impl
 * \ingroup libm_impl
 * \file
 * \endcond
 */

struct picotm_error;

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
void
fpu_tx_init(struct fpu_tx* self, unsigned long module,
            struct picotm_error* error);

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
void
fpu_tx_save_fenv(struct fpu_tx* self, struct picotm_error* error);

/**
 * Saves the current thread-local floating-point status flags.
 */
void
fpu_tx_save_fexcept(struct fpu_tx* self, struct picotm_error* error);

/**
 * Reverts all transaction-local changes.
 */
void
fpu_tx_undo(struct fpu_tx* self, struct picotm_error* error);

/**
 * Cleans up a transaction's FPU state.
 */
void
fpu_tx_finish(struct fpu_tx* self);
