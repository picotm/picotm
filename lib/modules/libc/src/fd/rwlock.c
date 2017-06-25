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
