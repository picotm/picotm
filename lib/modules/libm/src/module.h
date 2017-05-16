/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * \cond impl || libm_impl
 * \ingroup libm_impl
 * \file
 * \endcond
 */

void
fpu_module_save_fenv(void);

void
fpu_module_save_fexcept(void);

/**
 * \cond impl || libm_impl
 * \defgroup libm_impl libm Implementation
 * \endcond
 */
