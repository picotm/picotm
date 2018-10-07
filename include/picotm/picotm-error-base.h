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

/**
 * \ingroup group_core
 * \file
 *
 * \brief Contains picotm error constants.
 */

#include "picotm/config/picotm-config.h"
#include "compiler.h"

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_core
 * Signals detected errors to picotm.
 *
 * If a component detects an error, it should prefer setting the system-
 * specific error code that was returned by the failed call. This is more
 * specific than the generic one's below. In some portable code, such as
 * the tm module, using the generic codes might be preferable.
 */
enum picotm_error_code {
    /** The exact error is unknown. */
    PICOTM_GENERAL_ERROR = 0,
    /** Out-of-Memory error. */
    PICOTM_OUT_OF_MEMORY,
    /** Invalid floating-point environment. */
    PICOTM_INVALID_FENV,
    /** Out-of-Bounds memory access. */
    PICOTM_OUT_OF_BOUNDS
};

/**
 * \ingroup group_core
 * Signals error status to picotm.
 */
enum picotm_error_status {
    /** Conflict among transactions detected. */
    PICOTM_CONFLICTING = 1,
    /** Transaction requires irrevocability to continue. */
    PICOTM_REVOCABLE,
    /** Error detected. Encoded as `enum picotm_error_code`. */
    PICOTM_ERROR_CODE,
    /** Error detected. Encoded as errno code. */
    PICOTM_ERRNO,
    /** Error detected. Encoded as `kern_return_t` value. */
    PICOTM_KERN_RETURN_T,
    /** Error detected. Encoded as signal's `siginfo_t` value. */
    PICOTM_SIGINFO_T
};

PICOTM_END_DECLS
