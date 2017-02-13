/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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

