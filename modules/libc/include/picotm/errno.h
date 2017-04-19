/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <errno.h>
#include <picotm/compiler.h>

PICOTM_BEGIN_DECLS

PICOTM_NOTHROW
/**
 * Returns the address of 'errno'.
 * \warning This is an internal interface. Don't use it in application code.
 */
int*
__errno_location_tx(void);

/**
 * A transaction-safe implementation of 'errno'.
 */
#define errno_tx    (*(__errno_location_tx()))

PICOTM_END_DECLS
