/*
 * picotm - A system-level transaction manager
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
