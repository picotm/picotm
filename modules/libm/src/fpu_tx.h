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
