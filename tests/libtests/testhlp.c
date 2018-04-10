/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <tdz@users.sourceforge.net>
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
