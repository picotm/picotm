/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/compiler.h>
#include <sched.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sched.h>.
 */

PICOTM_NOTHROW
/**
 * A transaction-safe implementation of [sched_yield()][posix::sched_yield].
 *
 * [posix::sched_yield]:
 *  http://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_yield.html
 */
int
sched_yield_tx(void);

PICOTM_END_DECLS
