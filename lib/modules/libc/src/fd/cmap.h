/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMAP_H
#define CMAP_H

#include <pthread.h>
#include "pgtree.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct cmap_page
{
    pthread_mutex_t    lock;
    unsigned long long count[PGTREE_NENTRIES];
};

int
cmap_page_lock(struct cmap_page *pg);

void
cmap_page_unlock(struct cmap_page *pg);

struct cmap
{
    struct pgtree super;
};

int
cmap_init(struct cmap *cmap);

void
cmap_uninit(struct cmap *cmap);

struct cmap_page *
cmap_lookup_page(struct cmap *cmap, unsigned long long offset);

#endif

