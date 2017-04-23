/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

