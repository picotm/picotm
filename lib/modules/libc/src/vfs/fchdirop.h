/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FCHDIROP_H
#define FCHDIROP_H

/**
 * \cond impl || libc_impl || libc_impl_vfs
 * \ingroup libc_impl
 * \ingroup libc_impl_vfs
 * \file
 * \endcond
 */

struct fchdirop
{
    int oldcwd;
    int newcwd;
};

int
fchdirop_init(struct fchdirop *fchdirop, int oldcwd, int newfwd);

void
fchdirop_dump(struct fchdirop *fchdirop);

#endif

