/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testhlp.h"
#include <systx/stdlib.h>
#include <systx/systx-tm.h>

/**
 * Delays a transaction while generating potential conflicts.
 */
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
