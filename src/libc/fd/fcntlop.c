/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "types.h"
#include "fcntlop.h"

int
fcntlop_init(struct fcntlop *fcntlop, int command, const union com_fd_fcntl_arg *value,
                                                   const union com_fd_fcntl_arg *oldvalue)
{
    assert(fcntlop);

    fcntlop->command = command;

    if (value) {
        memcpy(&fcntlop->value, value, sizeof(fcntlop->value));
    }
    if (oldvalue) {
        memcpy(&fcntlop->oldvalue, oldvalue, sizeof(fcntlop->oldvalue));
    }

    return 0;
}

void
fcntlop_dump(struct fcntlop *fcntlop)
{
    fprintf(stderr, "fcntlop %p %d %d %d", (void*)fcntlop, fcntlop->command,
                                                           fcntlop->value.arg0,
                                                           fcntlop->oldvalue.arg0);
}

