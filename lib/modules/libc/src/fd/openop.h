/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OPENOP_H
#define OPENOP_H

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct openop
{
    unsigned char unlink;
};

int
openop_init(struct openop *openop, int unlink);

void
openop_dump(struct openop *openop);

#endif

