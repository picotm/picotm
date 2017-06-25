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

#ifndef RWLOCK_H
#define RWLOCK_H

#include <pthread.h>
#include <stdbool.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;

enum rwstate {
    RW_NOLOCK = 0,
    RW_RDLOCK = 0x1,
    RW_WRLOCK = 0x2,
    RW_BOTH   = RW_RDLOCK|RW_WRLOCK,
};

struct rwlock
{
    volatile pthread_spinlock_t lock;
    pthread_t          wr;
    unsigned int       n;
};

void
rwlock_init(struct rwlock *rwlock, struct picotm_error* error);

void
rwlock_uninit(struct rwlock *rwlock);

bool
rwlock_rdlock(struct rwlock *rwlock, int upgrade, struct picotm_error* error);

void
rwlock_rdunlock(struct rwlock *rwlock);

bool
rwlock_wrlock(struct rwlock *rwlock, int upgrade, struct picotm_error* error);

void
rwlock_wrunlock(struct rwlock *rwlock);

#endif

