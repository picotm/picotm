/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FCNTLOP_H
#define FCNTLOP_H

union com_fd_fcntl_arg
{
    int          arg0;
    struct flock arg1;
};

struct fcntlop
{
    int command;
    union com_fd_fcntl_arg value;
    union com_fd_fcntl_arg oldvalue;
};

int
fcntlop_init(struct fcntlop *fcntlop, int command, const union com_fd_fcntl_arg *value,
                                                   const union com_fd_fcntl_arg *oldvalue);

void
fcntlop_dump(struct fcntlop *fcntlop);

#endif

