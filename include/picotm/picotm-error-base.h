/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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
