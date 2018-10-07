/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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
 *
 * SPDX-License-Identifier: MIT
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
