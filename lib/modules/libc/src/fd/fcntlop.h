/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FCNTLOP_H
#define FCNTLOP_H

#include <fcntl.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

union fcntl_arg
{
    int          arg0;
    struct flock arg1;
};

struct fcntlop
{
    int command;
    union fcntl_arg value;
    union fcntl_arg oldvalue;
};

void
fcntlop_init(struct fcntlop *fcntlop, int command,
             const union fcntl_arg *value,
             const union fcntl_arg *oldvalue);

void
fcntlop_dump(struct fcntlop *fcntlop);

#endif

