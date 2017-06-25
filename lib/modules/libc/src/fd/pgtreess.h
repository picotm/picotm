/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

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

