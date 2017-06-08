/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

