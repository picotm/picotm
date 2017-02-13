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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "spinlock.h"

int
spinlock_init(pthread_spinlock_t *lock, int pshared)
{
    int err = pthread_spin_init(lock, pshared);

    if (err) {
        errno = err;
        perror("pthread_spin_init");
        return -1;
    }

    return 0;
}

int
spinlock_uninit(pthread_spinlock_t *lock)
{
    int err = pthread_spin_destroy(lock);

    if (err) {
        errno = err;
        perror("pthread_spin_uninit");
        return -1;
    }

    return 0;
}

void
spinlock_lock(pthread_spinlock_t *lock)
{
    int err = pthread_spin_lock(lock);

    if (err) {
        errno = err;
        perror("pthread_spin_lock");
        abort();
    }
}

void
spinlock_unlock(pthread_spinlock_t *lock)
{
    int err = pthread_spin_unlock(lock);

    if (err) {
        errno = err;
        perror("pthread_spin_unlock");
        abort();
    }
}

