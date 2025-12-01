/*
 * picotm - A system-level transaction manager
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

#include "testhlp.h"
#include "picotm/picotm.h"
#include "picotm/picotm-module.h"
#include <sched.h>
#include <string.h>
#include "safeblk.h"
#include "taputils.h"

void
abort_transaction_on_error(const char* origin)
{
    switch (picotm_error_status()) {
        case PICOTM_CONFLICTING:
            tap_error("%s: Conflicting transactions\n", origin);
            break;
        case PICOTM_REVOCABLE:
            tap_error("%s: Irrevocability required\n", origin);
            break;
        case PICOTM_ERROR_CODE: {
            enum picotm_error_code error_code =
                picotm_error_as_error_code();
            tap_error("%s: Error code %d\n", origin, (int)error_code);
            break;
        }
        case PICOTM_ERRNO: {
            int errno_code = picotm_error_as_errno();
            tap_error("%s: Errno code %d (%s)\n", origin, errno_code,
                      strerror(errno_code));
            break;
        }
        default:
            tap_error("%s, No error detected.", origin);
    }

    abort_safe_block();
}

void
delay_transaction_tx(unsigned int tid)
{
    static const unsigned int MIN_NYIELD = 10;
    static const unsigned int RND_NYIELD = 7;

    unsigned int nyield = MIN_NYIELD + (tid % RND_NYIELD);

    /* Switch threads several times to provoke conflicts with
     * concurrent transactions. */

    for (; nyield; --nyield) {
        int err = sched_yield();
        if (err) {
            tap_error("%s:%d: sched_yield failed: %d (%s)",
                      __func__, __LINE__, err, strerror(err));
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        }
    }
}
