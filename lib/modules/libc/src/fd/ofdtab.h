/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OFDTAB_H
#define OFDTAB_H

#include "ofd.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

extern struct ofd ofdtab[MAXNUMFD];

void
ofdtab_lock(void);

void
ofdtab_unlock(void);

struct ofd*
ofdtab_ref_fildes(int fildes, bool want_new, bool unlink_file,
                  struct picotm_error* error);

size_t
ofdtab_index(struct ofd *ofd);

#endif

