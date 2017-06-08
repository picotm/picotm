/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "rwlock.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>

void
rwlock_init(struct rwlock* rwlock, struct picotm_error* error)
{
    assert(rwlock);

    int err = pthread_spin_init(&rwlock->lock, PTHREAD_PROCESS_PRIVATE);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    rwlock->wr = (pthread_t)0;
    rwlock->n = 0;
}

void
rwlock_uninit(struct rwlock *rwlock)
{
    assert(rwlock);

    rwlock->wr = (pthread_t)0;
    rwlock->n = 0;
    pthread_spin_destroy(&rwlock->lock);
}

bool
rwlock_rdlock(struct rwlock *rwlock, int upgrade, struct picotm_error* error)
{
    assert(rwlock);

    bool succ = false;

    pthread_spin_lock(&rwlock->lock);

    /* writer present */
    if ((rwlock->wr != (pthread_t)0) && (rwlock->wr != pthread_self())) {
        picotm_error_set_conflicting(error, NULL);
        goto unlock;
    }

    if (!upgrade) {
        ++rwlock->n;
    }

    succ = true;

unlock:
    pthread_spin_unlock(&rwlock->lock);
    return succ;
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

bool
rwlock_wrlock(struct rwlock *rwlock, int upgrade, struct picotm_error* error)
{
    assert(rwlock);

    bool succ = false;

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
            picotm_error_set_conflicting(error, NULL);
            goto unlock;
        }
    } else {
        /* another writer present */
        picotm_error_set_conflicting(error, NULL);
        goto unlock;
    }

    succ = true;

unlock:
    pthread_spin_unlock(&rwlock->lock);
    return succ;
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
