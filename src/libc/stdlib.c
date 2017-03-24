/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <systx/systx.h>
#include "systx/stdlib.h"

/*
 * Programm termination
 */

SYSTX_EXPORT
void
abort_tx()
{
    systx_commit();
    abort();
}

SYSTX_EXPORT
void
exit_tx(int status)
{
    systx_commit();
    exit(status);
}

SYSTX_EXPORT
void
_Exit_tx(int status)
{
    systx_commit();
    _Exit(status);
}
