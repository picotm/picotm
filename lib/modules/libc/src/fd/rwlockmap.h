/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RWLOCKMAP_H
#define RWLOCKMAP_H

#include "pgtree.h"

struct rwlockmap_page
{
    unsigned long lock[PGTREE_NENTRIES];
};

struct rwlockmap
{
    struct pgtree super;
};

int
rwlockmap_init(struct rwlockmap *rwlockmap);

void
rwlockmap_uninit(struct rwlockmap *rwlockmap);

struct rwlockmap_page *
rwlockmap_lookup_page(struct rwlockmap *rwlockmap, unsigned long long offset);

#endif

