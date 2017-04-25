/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PGTREESS_H
#define PGTREESS_H

#include <stddef.h>

union pgtreess_dir_entry
{
    struct pgtreess_dir *dir;
    void                *any;
};

struct pgtreess
{
    size_t                   ndirs;
    union pgtreess_dir_entry entry;
};

int
pgtreess_init(struct pgtreess *pgtreess);

void
pgtreess_uninit(struct pgtreess *pgtreess, void (*destroy_page_fn)(void*));

void *
pgtreess_lookup_page(struct pgtreess *pgtreess,
                     unsigned long long offset, void* (*create_page_fn)(void));

int
pgtreess_for_each_page(struct pgtreess *pgtreess,
                       int (*page_fn)(void*, unsigned long long, void*),
                       void *data);

#endif

