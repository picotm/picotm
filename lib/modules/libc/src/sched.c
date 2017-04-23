/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/sched.h"

PICOTM_EXPORT
int
sched_yield_tx()
{
    /* Always succeeds in practice */
    return sched_yield();
}
