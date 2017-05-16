/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RWLOCK_H
#define RWLOCK_H

#include <pthread.h>
#include "errcode.h"

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

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

int
rwlock_init(struct rwlock *rwlock);

void
rwlock_uninit(struct rwlock *rwlock);

enum error_code
rwlock_rdlock(struct rwlock *rwlock, int upgrade);

void
rwlock_rdunlock(struct rwlock *rwlock);

enum error_code
rwlock_wrlock(struct rwlock *rwlock, int upgrade);

void
rwlock_wrunlock(struct rwlock *rwlock);

int
rwlock_init_walk(void *rwlock);

int
rwlock_uninit_walk(void *rwlock);

#endif

