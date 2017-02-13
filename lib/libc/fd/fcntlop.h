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

