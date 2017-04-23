/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

