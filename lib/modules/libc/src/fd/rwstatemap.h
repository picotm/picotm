/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RWSTATEMAP_H
#define RWSTATEMAP_H

#include <stdbool.h>
#include "pgtreess.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct rwlockmap;

struct rwstatemap
{
    struct pgtreess super;
};

void
rwstatemap_init(struct rwstatemap *rwstatemap);

void
rwstatemap_uninit(struct rwstatemap *rwstatemap);

bool
rwstatemap_rdlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap,
                                                 struct picotm_error* error);

bool
rwstatemap_wrlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap,
                                                 struct picotm_error* error);

void
rwstatemap_unlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap,
                                                 struct picotm_error* error);

/*int
rwstatemap_unlock_all(struct rwstatemap *rwstatemap,
                      struct rwlockmap *rwlockmap);*/

#endif

