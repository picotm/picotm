/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "picotm/config/picotm-libm-config.h"
#include "picotm/compiler.h"

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
 * \ingroup group_libm
 * Saves the floating-point environment.
 */
void
picotm_libm_save_fenv(void);

PICOTM_NOTHROW
/**
 * \ingroup group_libm
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
 * of typical C math functions, such as `sqrt()` or `pow()`.
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
