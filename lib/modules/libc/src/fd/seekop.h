/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SEEKOP_H
#define SEEKOP_H

#include <sys/types.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct seekop
{
    off_t from;
    off_t offset;
    int   whence;
};

void
seekop_init(struct seekop *seekop, off_t from, off_t offset, int whence);

void
seekop_dump(struct seekop *seekop);

#endif

