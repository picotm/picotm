/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>

PICOTM_BEGIN_DECLS

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
