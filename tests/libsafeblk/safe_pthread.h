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
#include "compat/pthread_barrier.h"

int
safe_pthread_barrier_destroy(pthread_barrier_t* barrier);

int
safe_pthread_barrier_init(pthread_barrier_t* restrict barrier,
                          const pthread_barrierattr_t* restrict attr,
                          unsigned count);

int
safe_pthread_barrier_wait(pthread_barrier_t* barrier);

int
safe_pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                    void* (*start_routine)(void*), void* arg);

int
safe_pthread_join(pthread_t thread, void** retval);

int
safe_pthread_mutex_lock(pthread_mutex_t* mutex);

int
safe_pthread_mutex_unlock(pthread_mutex_t* mutex);
