/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RWSTATEMAP_H
#define RWSTATEMAP_H

#include "pgtreess.h"

struct rwlockmap;

struct rwstatemap
{
    struct pgtreess super;
};

int
rwstatemap_init(struct rwstatemap *rwstatemap);

void
rwstatemap_uninit(struct rwstatemap *rwstatemap);

int
rwstatemap_rdlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap);

int
rwstatemap_wrlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap);

int
rwstatemap_unlock(struct rwstatemap *rwstatemap, unsigned long long length,
                                                 unsigned long long offset,
                                                 struct rwlockmap *rwlockmap);

/*int
rwstatemap_unlock_all(struct rwstatemap *rwstatemap,
                      struct rwlockmap *rwlockmap);*/

#endif

