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

#ifndef SPINLOCK_H
#define SPINLOCK_H

int
spinlock_init(pthread_spinlock_t *lock, int pshared);

int
spinlock_uninit(pthread_spinlock_t *lock);

void
spinlock_lock(pthread_spinlock_t *lock);

void
spinlock_unlock(pthread_spinlock_t *lock);

#endif

