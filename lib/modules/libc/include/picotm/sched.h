/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <sched.h>

PICOTM_BEGIN_DECLS

PICOTM_NOTHROW
/**
 * Executes sched_yield() within a transaction.
 */
int
sched_yield_tx(void);

PICOTM_END_DECLS
