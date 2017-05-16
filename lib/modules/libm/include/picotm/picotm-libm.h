/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libm
 * \file
 *
 * \brief Public interfaces of picotm's libm module.
 *
 * The public interfaces of the libm module allow for saving the
 * floating-point environment and status flags. This should usually
 * not be done from within the transaction itself. Instead use these
 * functions from within other modules if they modify these settings.
 */

PICOTM_NOTHROW
/**
 * Saves the floating-point environment.
 */
void
picotm_libm_save_fenv(void);

PICOTM_NOTHROW
/**
 * Saves the floating-point status flags.
 */
void
picotm_libm_save_fexcept(void);

PICOTM_END_DECLS

/**
 * \defgroup group_libm The C Math Library Module
 *
 * \brief Covers the C Math Library module. This module provides
 *        transactional C math functions, such `sqrt()` or `pow()`.
 *
 * The C Math Library module provides transactional implementations
 * of typical C math functions, such ar `sqrt()` or `pow()`.
 * The functions in this module can simply be invoked without further
 * configuration. Most of them don't have any state besides error reporting
 * and the floating-point environment. All functions instrument the TM
 * module for privatizing argument buffers, and instrument libc for saving
 * errno.
 *
 * The C Math Library maintains two global states. These are the
 * floating-point environment and the floating-point status flags.
 *
 * The floating-point environment contains configuration flags for operations
 * that involve floating-point arithmetics. A typical example is the rounding
 * mode.
 *
 * The floating-point status flags signal errors during floating-point
 * operations. Examples are overflow errors or divisions by zero.
 *
 * All functions in libm handle these state variables automatically. If you
 * implement an external module that modifies these states, libm offers two
 * interface for saving the states. Call picotm_libm_save_fenv() to save the
 * floating-point environment, and call picotm_libm_save_fexcept() to save the
 * floating-point status flags. This will save and restore each state when
 * necessary.
 */
