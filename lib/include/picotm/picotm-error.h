/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

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
    PICOTM_OUT_OF_MEMORY
};

PICOTM_END_DECLS
