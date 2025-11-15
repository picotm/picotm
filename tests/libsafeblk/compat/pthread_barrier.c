/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "pthread_barrier.h"
#include <assert.h>
#include <errno.h>

int
__picotm_pthread_barrier_init(
    __picotm_pthread_barrier_t* barrier,
    const __picotm_pthread_barrierattr_t* restrict attr,
    unsigned count)
{
    assert(barrier);
    assert(!attr); // we don't support attributes

    if (!count) {
        return EINVAL;
    }

    int err = pthread_mutex_init(&barrier->mutex, NULL);
    if (err) {
        return err;
    }
    err = pthread_cond_init(&barrier->cond, NULL);
    if (err) {
        goto err_pthread_cond_init;
    }

    barrier->count = count;

    return 0;

err_pthread_cond_init:
    pthread_mutex_destroy(&barrier->mutex);
    return err;
}

int
__picotm_pthread_barrier_destroy(__picotm_pthread_barrier_t* barrier)
{
    assert(barrier);

    int err = pthread_mutex_destroy(&barrier->mutex);
    if (err) {
        return err;
    }
    err = pthread_cond_destroy(&barrier->cond);
    if (err) {
        return err;
    }
    return 0;
}

int
__picotm_pthread_barrier_wait(__picotm_pthread_barrier_t* barrier)
{
    assert(barrier);

    int err = pthread_mutex_lock(&barrier->mutex);
    if (err) {
        return err;
    }

    --barrier->count;

    int res;

    if (barrier->count) {
        err = pthread_cond_wait(&barrier->cond, &barrier->mutex);
        if (err) {
            goto err_pthread_cond_op;
        }
        res = 0;
    } else {
        err = pthread_cond_broadcast(&barrier->cond);
        if (err) {
            goto err_pthread_cond_op;
        }
        res = __PICOTM_PTHREAD_BARRIER_SERIAL_THREAD;
    }

    ++barrier->count;

    err = pthread_mutex_unlock(&barrier->mutex);
    if (err) {
        goto err_pthread_mutex_unlock;
    }

    return res;

err_pthread_cond_op:
    pthread_mutex_unlock(&barrier->mutex);
err_pthread_mutex_unlock:
    return err;
}
