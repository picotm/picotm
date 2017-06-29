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

#ifndef RWSTATEMAP_H
#define RWSTATEMAP_H

#include <picotm/picotm-lib-treemap.h>
#include <stdbool.h>

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
    struct picotm_treemap super;
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

