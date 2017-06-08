/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RWLOCKMAP_H
#define RWLOCKMAP_H

#include <stdatomic.h>
#include "pgtree.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

struct rwlockmap_page
{
    atomic_ulong lock[PGTREE_NENTRIES];
};

struct rwlockmap
{
    struct pgtree super;
};

void
rwlockmap_init(struct rwlockmap* rwlockmap, struct picotm_error* error);

void
rwlockmap_uninit(struct rwlockmap *rwlockmap);

struct rwlockmap_page*
rwlockmap_lookup_page(struct rwlockmap* rwlockmap, unsigned long long offset,
                      struct picotm_error* error);

#endif

