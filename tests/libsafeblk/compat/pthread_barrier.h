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

#pragma once

#include <pthread.h>

#define __PICOTM_PTHREAD_BARRIER_SERIAL_THREAD  (-1)

#ifndef PTHREAD_BARRIER_SERIAL_THREAD
#define PTHREAD_BARRIER_SERIAL_THREAD   __PICOTM_PTHREAD_BARRIER_SERIAL_THREAD
#endif

typedef struct {
    int filler[0];
} __picotm_pthread_barrierattr_t;

#if !defined(HAVE_PTHREAD_BARRIERATTR_T) || !HAVE_PTHREAD_BARRIERATTR_T
typedef __picotm_pthread_barrierattr_t pthread_barrierattr_t;
#endif

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned count;
} __picotm_pthread_barrier_t;

int
__picotm_pthread_barrier_init(
    __picotm_pthread_barrier_t* barrier,
    const __picotm_pthread_barrierattr_t* restrict attr,
    unsigned count);

int
__picotm_pthread_barrier_destroy(__picotm_pthread_barrier_t* barrier);

int
__picotm_pthread_barrier_wait(__picotm_pthread_barrier_t* barrier);

#if !defined(HAVE_PTHREAD_BARRIER_T) || !HAVE_PTHREAD_BARRIER_T
typedef __picotm_pthread_barrier_t pthread_barrier_t;
#define pthread_barrier_init    __picotm_pthread_barrier_init
#define pthread_barrier_destroy __picotm_pthread_barrier_destroy
#define pthread_barrier_wait    __picotm_pthread_barrier_wait
#endif
