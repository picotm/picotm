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
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <picotm/compiler.h>
#include <picotm/config/picotm-libm-config.h>

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
