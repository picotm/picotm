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

#ifndef MUTEX_H
#define MUTEX_H

int
mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

int
mutex_uninit(pthread_mutex_t *mutex);

void
mutex_lock(pthread_mutex_t *mutex);

void
mutex_unlock(pthread_mutex_t *mutex);

#endif

