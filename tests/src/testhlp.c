/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testhlp.h"
#include <picotm/picotm.h>
#include <picotm/picotm-tm.h>
#include <picotm/stdlib.h>
#include <stdio.h>
#include <string.h>

void
delay_transaction(unsigned int tid)
{
    static unsigned int g_curtid;

	store_uint_tx(&g_curtid, tid);

	for (int i = 0; i < 10000; ++i) {
		unsigned int curtid = load_uint_tx(&g_curtid);
        if (curtid != tid) {
		    abort_tx();
		}
	}
}

void
abort_transaction_on_error(const char* origin)
{
    switch (picotm_error_status()) {
        case PICOTM_CONFLICTING:
            fprintf(stderr, "%s: Conficting transactions\n", origin);
            break;
        case PICOTM_REVOCABLE:
            fprintf(stderr, "%s: Irrevocability required\n", origin);
            break;
        case PICOTM_ERROR_CODE: {
            enum picotm_error_code error_code =
                picotm_error_as_error_code();
            fprintf(stderr, "%s: Error code %d\n", origin,
                    (int)error_code);
            break;
        }
        case PICOTM_ERRNO: {
            int errno_code = picotm_error_as_errno();
            fprintf(stderr, "%s: Errno code %d (%s)\n", origin,
                    errno_code, strerror(errno_code));
            break;
        }
        default:
            fprintf(stderr, "%s, No error detected.", origin);
    }

    abort();
}
