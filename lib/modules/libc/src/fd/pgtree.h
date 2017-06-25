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

#ifndef PGTREE_H
#define PGTREE_H

#include <pthread.h>
#include <stddef.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

enum {
    PGTREE_ENTRY_NBITS = 9,
    PGTREE_ENTRY_MASK  = 0x1ff,
    PGTREE_NENTRIES    = (0x1 << PGTREE_ENTRY_NBITS)
};

struct pgtree_dir;
struct picotm_error;

struct pgtree_dir_entry
{
    pthread_spinlock_t  lock;
    union {
        struct pgtree_dir *dir;
        void              *any;
    } data;
};

struct pgtree_dir
{
    struct pgtree_dir_entry entry[PGTREE_NENTRIES];
};

struct pgtree
{
    size_t                  ndirs;
    struct pgtree_dir_entry entry;
};

void
pgtree_init(struct pgtree *pgtree, struct picotm_error* error);

void
pgtree_uninit(struct pgtree *pgtree, void (*pg_destroy_fn)(void*));

void*
pgtree_lookup_page(struct pgtree *pgtree, unsigned long long offset,
                   void* (*pg_create_fn)(struct picotm_error*),
                   struct picotm_error* error);

#endif

