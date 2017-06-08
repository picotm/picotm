/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PGTREESS_H
#define PGTREESS_H

#include <stddef.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

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

void
pgtreess_init(struct pgtreess *pgtreess);

void
pgtreess_uninit(struct pgtreess *pgtreess, void (*destroy_page_fn)(void*));

void*
pgtreess_lookup_page(struct pgtreess *pgtreess, unsigned long long offset,
                     void* (*create_page_fn)(struct picotm_error*),
                     struct picotm_error* error);

void
pgtreess_for_each_page(struct pgtreess *pgtreess,
                       void (*page_fn)(void*, unsigned long long, void*,
                                       struct picotm_error*),
                       void* data, struct picotm_error* error);

#endif

