/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef PGTREESS_H
#define PGTREESS_H

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

