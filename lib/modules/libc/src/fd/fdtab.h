/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FDTAB_H
#define FDTAB_H

#include "fd.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

extern struct fd fdtab[MAXNUMFD];

/*int
fdtab_init(void);*/

struct fd*
fdtab_ref_fildes(int fildes, bool want_new, struct picotm_error* error);

#endif

