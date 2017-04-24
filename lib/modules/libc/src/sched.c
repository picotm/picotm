/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "picotm/sched.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "picotm/picotm-libc.h"

PICOTM_EXPORT
int
sched_yield_tx()
{
    picotm_libc_save_errno();

    int res;

    do {
        res = sched_yield();
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}
