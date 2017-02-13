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

#ifndef RWLOCK_H
#define RWLOCK_H

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

