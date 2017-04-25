/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rwlock.h"
#include <assert.h>
#include <errno.h>

int
rwlock_init(struct rwlock *rwlock)
{
    int err;

    assert(rwlock);

    if ((err = pthread_spin_init(&rwlock->lock, PTHREAD_PROCESS_PRIVATE))) {
        errno = err;
        return ERR_SYSTEM;
    }

    rwlock->wr = (pthread_t)0;
    rwlock->n = 0;

    return 0;
}

void
rwlock_uninit(struct rwlock *rwlock)
{
    assert(rwlock);

    rwlock->wr = (pthread_t)0;
    rwlock->n = 0;
    pthread_spin_destroy(&rwlock->lock);
}

enum error_code
rwlock_rdlock(struct rwlock *rwlock, int upgrade)
{
    assert(rwlock);

    pthread_spin_lock(&rwlock->lock);

    /* writer present */
    if ((rwlock->wr != (pthread_t)0) && (rwlock->wr != pthread_self())) {
        pthread_spin_unlock(&rwlock->lock);
        return ERR_CONFLICT;
    }

    if (!upgrade) {
        ++rwlock->n;
    }

    pthread_spin_unlock(&rwlock->lock);

    return 0;
}

void
rwlock_rdunlock(struct rwlock *rwlock)
{
    assert(rwlock);
    assert(rwlock->n);

    pthread_spin_lock(&rwlock->lock);
    --rwlock->n;
    pthread_spin_unlock(&rwlock->lock);
}

enum error_code
rwlock_wrlock(struct rwlock *rwlock, int upgrade)
{
    assert(rwlock);

    const pthread_t self = pthread_self();

    pthread_spin_lock(&rwlock->lock);

    if (rwlock->wr == self) {
        /* self writer */
        ++rwlock->n;
    } else if (rwlock->wr == (pthread_t)0) {
        /* no writer present */
        if (!rwlock->n) {
            /* no readers at all */
            rwlock->wr = self;
            ++rwlock->n;
        } else if ((rwlock->n==1) && upgrade) {
            /* upgrade from reader to writer */
            rwlock->wr = self;
        } else {
            /* other readers present */
            pthread_spin_unlock(&rwlock->lock);
            return ERR_CONFLICT;
        }
    } else {
        /* another writer present */
        pthread_spin_unlock(&rwlock->lock);
        return ERR_CONFLICT;
    }

    pthread_spin_unlock(&rwlock->lock);

    return 0;
}

void
rwlock_wrunlock(struct rwlock *rwlock)
{
    assert(rwlock);
    assert(rwlock->wr == pthread_self());
    assert(rwlock->n);

    pthread_spin_lock(&rwlock->lock);

    --rwlock->n;
    rwlock->wr = (pthread_t)0;

    pthread_spin_unlock(&rwlock->lock);
}

int
rwlock_init_walk(void *rwlock)
{
    return rwlock_init(rwlock) < 0 ? -1 : 1;
}

int
rwlock_uninit_walk(void *rwlock)
{
    rwlock_uninit(rwlock);

    return 1;
}

