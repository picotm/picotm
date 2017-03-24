/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systx/unistd.h"
#include <systx/systx.h>

SYSTX_EXPORT
void
_exit_tx(int status)
{
    systx_commit();
    _exit(status);
}
